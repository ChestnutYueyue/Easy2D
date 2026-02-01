const fs = require('fs');
const path = require('path');

const srcPath = 'Easy2D/src';

const oldDirs = [
    'Action',
    'Base',
    'Common',
    'Manager',
    'Node',
    'OpenGL',
    'Tool',
    'Transition',
    'Math'
];

function removeDirectory(dirPath) {
    try {
        if (fs.existsSync(dirPath)) {
            const files = fs.readdirSync(dirPath);
            
            if (files.length === 0) {
                fs.rmdirSync(dirPath);
                console.log(`删除空目录: ${dirPath}`);
                return true;
            } else {
                console.log(`跳过(非空): ${dirPath}`);
                return false;
            }
        } else {
            console.log(`跳过(不存在): ${dirPath}`);
            return false;
        }
    } catch (error) {
        console.error(`删除失败: ${dirPath} - ${error.message}`);
        return false;
    }
}

function main() {
    console.log('开始清理旧目录...\n');
    
    let deletedCount = 0;
    let skippedCount = 0;
    
    for (const dir of oldDirs) {
        const dirPath = path.join(srcPath, dir);
        if (removeDirectory(dirPath)) {
            deletedCount++;
        } else {
            skippedCount++;
        }
    }
    
    console.log(`\n清理完成!`);
    console.log(`成功删除: ${deletedCount} 个目录`);
    console.log(`跳过: ${skippedCount} 个目录`);
}

main();
