// player_controller.nut
// 精灵动画演示脚本 —— 复刻 animation_demo 功能
// 使用引擎 AnimationClip::createFromGrid 从精灵图创建动画
// 使用 WASD 或方向键控制角色移动

// 方向枚举
// 精灵图布局（96x96像素每帧，4x4=16帧）：
// 第1行 (索引 0-3):  S 向下走（正面，人物朝向你）
// 第2行 (索引 4-7):  A 向左走（左侧面）
// 第3行 (索引 8-11): D 向右走（右侧面）
// 第4行 (索引 12-15): W 向上走（背面，人物背对你）
local Direction = {
    Down = 0,   // S 向下走 - 帧 0-3（人物正面朝向你）
    Left = 1,   // A 向左走 - 帧 4-7
    Right = 2,  // D 向右走 - 帧 8-11
    Up = 3      // W 向上走 - 帧 12-15（人物背对你）
}

// 配置
local kFrameWidth = 96
local kFrameHeight = 96
local kFramesPerDir = 4
local moveSpeed = 150.0
local frameDurationMs = 125.0  // 8 FPS

return {
    character = null
    currentDir = Direction.Down  // 初始方向：向下（人物正面朝向你）
    isMoving = false

    function onEnter(node) {
        log("PlayerController: onEnter")

        // 创建动画精灵
        // 从精灵图网格创建（96x96像素每帧，4x4=16帧）
        // 第1行 (索引 0-3):  S 向下走（正面）
        // 第2行 (索引 4-7):  A 向左走
        // 第3行 (索引 8-11): D 向右走
        // 第4行 (索引 12-15): W 向上走
        character = AnimatedSprite.createFromGrid("player.png", kFrameWidth, kFrameHeight, frameDurationMs, 16)

        if (character == null) {
            log("Failed to create AnimatedSprite!")
            return
        }

        // 设置初始帧范围（S向下走：帧 0-3，人物正面朝向你）
        character.setFrameRange(0, 3)
        character.setFrameIndex(0)
        character.setPosition(450.0, 300.0)

        node.addChild(character)

        log("PlayerController: character created at (450, 300)")
        log("Total frames: " + character.getTotalFrames())
    }

    function onUpdate(node, dt) {
        if (character == null) return

        isMoving = false

        // 处理输入
        if (Input.isKeyDown(Key.W) || Input.isKeyDown(Key.Up)) {
            moveCharacter(Direction.Up, dt)
        } else if (Input.isKeyDown(Key.S) || Input.isKeyDown(Key.Down)) {
            moveCharacter(Direction.Down, dt)
        } else if (Input.isKeyDown(Key.A) || Input.isKeyDown(Key.Left)) {
            moveCharacter(Direction.Left, dt)
        } else if (Input.isKeyDown(Key.D) || Input.isKeyDown(Key.Right)) {
            moveCharacter(Direction.Right, dt)
        }

        // 如果没有移动，暂停动画
        if (!isMoving && character.isPlaying()) {
            character.pause()
        }

        // ESC 退出
        if (Input.isKeyPressed(Key.Escape)) {
            App.quit()
        }
    }

    function moveCharacter(dir, dt) {
        // 根据方向计算对应的动画帧范围
        local newFrameStart = dir * kFramesPerDir
        local newFrameEnd = newFrameStart + kFramesPerDir - 1

        // 方向改变时，切换到新方向的帧范围
        if (currentDir != dir) {
            character.setFrameRange(newFrameStart, newFrameEnd)
            character.setFrameIndex(newFrameStart)
        }

        // 确保动画在播放
        if (!character.isPlaying()) {
            character.play()
        }

        currentDir = dir
        isMoving = true

        // 移动角色
        local pos = character.getPosition()
        local x = pos.getX()
        local y = pos.getY()

        switch (dir) {
            case Direction.Down:
                y = y + moveSpeed * dt
                break
            case Direction.Up:
                y = y - moveSpeed * dt
                break
            case Direction.Left:
                x = x - moveSpeed * dt
                break
            case Direction.Right:
                x = x + moveSpeed * dt
                break
        }

        character.setPosition(x, y)
    }

    function onExit(node) {
        log("PlayerController: onExit")
    }
}
