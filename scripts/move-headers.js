const fs = require('fs');
const path = require('path');

const srcPath = 'Easy2D/src';
const includePath = 'Easy2D/include/easy2d';

const headerFiles = [
    'Rendering/GLTexture.h',
    'Rendering/GLFrameBuffer.h',
    'Rendering/GLShader.h',
    'Rendering/GLBuffer.h',
    'Rendering/GLRenderer.h',
    'Rendering/GLTextRenderer.h'
];

function moveFile(oldPath, newPath) {
    const dir = path.dirname(newPath);
    if (!fs.existsSync(dir)) {
        fs.mkdirSync(dir, { recursive: true });
    }
    fs.renameSync(oldPath, newPath);
    console.log(`移动: ${oldPath} -> ${newPath}`);
}

function main() {
    console.log('开始移动.h文件到include目录...\n');
    
    let movedCount = 0;
    let errorCount = 0;
    
    for (const file of headerFiles) {
        const oldPath = path.join(srcPath, file);
        const fileName = path.basename(file);
        const newPath = path.join(includePath, fileName);
        
        try {
            if (fs.existsSync(oldPath)) {
                if (fs.existsSync(newPath)) {
                    console.log(`跳过(目标已存在): ${oldPath}`);
                    errorCount++;
                } else {
                    moveFile(oldPath, newPath);
                    movedCount++;
                }
            } else {
                console.log(`跳过(源文件不存在): ${oldPath}`);
                errorCount++;
            }
        } catch (error) {
            console.error(`错误: ${error.message}`);
            errorCount++;
        }
    }
    
    console.log(`\n移动完成!`);
    console.log(`成功移动: ${movedCount} 个文件`);
    console.log(`错误/跳过: ${errorCount} 个文件`);
}

main();
