const { spawn, spawnSync } = require("child_process");
const path = require("path");

function runPowerShell(command) {
  const result = spawnSync(
    "powershell.exe",
    ["-NoProfile", "-NonInteractive", "-Command", command],
    { encoding: "utf8" }
  );
  return {
    status: result.status ?? 1,
    stdout: (result.stdout || "").trim(),
    stderr: (result.stderr || "").trim(),
  };
}

function startApp(exePath, cwd) {
  const child = spawn(exePath, [], {
    cwd,
    stdio: "ignore",
    windowsHide: true,
    detached: true,
  });
  child.unref();
  return child.pid;
}

function closeMainWindowAndWait(pid, name) {
  const ps = [
    `$p = Get-Process -Id ${pid} -ErrorAction SilentlyContinue`,
    `if (-not $p) { Write-Output '${name}:process_not_found'; exit 1 }`,
    `Start-Sleep -Milliseconds 1500`,
    `$null = $p.CloseMainWindow()`,
    `$sw = [Diagnostics.Stopwatch]::StartNew()`,
    `$exited = $p.WaitForExit(15000)`,
    `$sw.Stop()`,
    `if ($exited) { Write-Output ('${name}:exited ms=' + $sw.ElapsedMilliseconds) } else { Write-Output ('${name}:timeout_kill ms=' + $sw.ElapsedMilliseconds); $p.Kill(); $p.WaitForExit() }`,
  ].join("; ");
  return runPowerShell(ps);
}

function main() {
  const root = __dirname;
  const wd = path.join(root, "GreedyMonster-Easy2D");

  const exes = [
    { name: "debug", exe: path.join(root, "build", "windows", "x64", "debug", "GreedyMonster.exe") },
    { name: "release", exe: path.join(root, "build", "windows", "x64", "release", "GreedyMonster.exe") },
  ];

  for (const it of exes) {
    const pid = startApp(it.exe, wd);
    const r = closeMainWindowAndWait(pid, it.name);
    if (r.stdout) process.stdout.write(r.stdout + "\n");
    if (r.status !== 0) {
      if (r.stderr) process.stderr.write(r.stderr + "\n");
      process.exitCode = 1;
    }
  }
}

main();
