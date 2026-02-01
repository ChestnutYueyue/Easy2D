const fs = require('fs');
const path = require('path');

const srcPath = 'Easy2D/src';

const headerMappings = {
    '../OpenGL/GLRenderer.h': '<easy2d/GLRenderer.h>',
    '../OpenGL/GLTexture.h': '<easy2d/GLTexture.h>',
    '../OpenGL/GLShader.h': '<easy2d/GLShader.h>',
    '../OpenGL/GLBuffer.h': '<easy2d/GLBuffer.h>',
    '../OpenGL/GLFrameBuffer.h': '<easy2d/GLFrameBuffer.h>',
    '../OpenGL/GLTextRenderer.h': '<easy2d/GLTextRenderer.h>',
    '../../OpenGL/GLTextRenderer.h': '<easy2d/GLTextRenderer.h>'
};

function fixHeaderIncludes(filePath) {
    try {
        let content = fs.readFileSync(filePath, 'utf8');
        let modified = false;

        for (const [oldInclude, newInclude] of Object.entries(headerMappings)) {
            const oldPattern = `#include "${oldInclude}"`;
            const newPattern = `#include ${newInclude}`;
            
            if (content.includes(oldPattern)) {
                content = content.replace(oldPattern, newPattern);
                modified = true;
                console.log(`  修复: ${oldPattern} -> ${newPattern}`);
            }
        }

        if (modified) {
            fs.writeFileSync(filePath, content, 'utf8');
            return true;
        }
        return false;
    } catch (error) {
        console.error(`错误处理文件 ${filePath}: ${error.message}`);
        return false;
    }
}

function findAndFixCppFiles(dir) {
    let fixedCount = 0;
    
    const files = fs.readdirSync(dir);
    
    for (const file of files) {
        const fullPath = path.join(dir, file);
        const stat = fs.statSync(fullPath);
        
        if (stat.isDirectory()) {
            fixedCount += findAndFixCppFiles(fullPath);
        } else if (file.endsWith('.cpp')) {
            console.log(`检查: ${fullPath}`);
            if (fixHeaderIncludes(fullPath)) {
                fixedCount++;
            }
        }
    }
    
    return fixedCount;
}

function main() {
    console.log('开始修复相对路径头文件引用...\n');
    
    const fixedCount = findAndFixCppFiles(srcPath);
    
    console.log(`\n修复完成!`);
    console.log(`修复了 ${fixedCount} 个文件`);
}

main();
