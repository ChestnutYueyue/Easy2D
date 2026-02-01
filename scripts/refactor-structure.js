const fs = require('fs');
const path = require('path');

const srcPath = 'Easy2D/src';

const fileMappings = {
    'Core': [
        'Base/Game.cpp',
        'Base/Window.cpp',
        'Base/Input.cpp',
        'Base/GC.cpp',
        'Base/Logger.cpp',
        'Base/Time.cpp'
    ],
    'Rendering': [
        'OpenGL/GLRenderer.cpp',
        'OpenGL/GLRenderer.h',
        'OpenGL/GLTexture.cpp',
        'OpenGL/GLTexture.h',
        'OpenGL/GLShader.cpp',
        'OpenGL/GLShader.h',
        'OpenGL/GLBuffer.cpp',
        'OpenGL/GLBuffer.h',
        'OpenGL/GLFrameBuffer.cpp',
        'OpenGL/GLFrameBuffer.h',
        'OpenGL/GLTextRenderer.cpp',
        'OpenGL/GLTextRenderer.h',
        'Base/Renderer.cpp'
    ],
    'Scene': [
        'Node/Node.cpp',
        'Node/Scene.cpp',
        'Node/Sprite.cpp',
        'Node/Text.cpp',
        'Node/Canvas.cpp',
        'Node/ShapeNode.cpp',
        'Node/Listener/Listener.cpp',
        'Node/Listener/ButtonListener.cpp',
        'Manager/SceneManager.cpp'
    ],
    'Animation': [
        'Action/Action.cpp',
        'Action/Animation.cpp',
        'Action/FiniteTimeAction.cpp',
        'Action/KeyFrame.cpp',
        'Action/FrameSequence.cpp',
        'Action/CallFunc.cpp',
        'Action/Delay.cpp',
        'Action/Loop.cpp',
        'Action/Sequence.cpp',
        'Action/Spawn.cpp',
        'Action/MoveBy.cpp',
        'Action/MoveTo.cpp',
        'Action/JumpBy.cpp',
        'Action/JumpTo.cpp',
        'Action/RotateBy.cpp',
        'Action/RotateTo.cpp',
        'Action/ScaleBy.cpp',
        'Action/ScaleTo.cpp',
        'Action/OpacityBy.cpp',
        'Action/OpacityTo.cpp',
        'Transition/Transition.cpp',
        'Transition/BoxTransition.cpp',
        'Transition/FadeTransition.cpp',
        'Transition/MoveTransition.cpp',
        'Manager/ActionManager.cpp'
    ],
    'Resources': [
        'Common/Image.cpp',
        'Common/Font.cpp',
        'Common/Resource.cpp',
        'Common/Color.cpp',
        'Common/DrawingStyle.cpp',
        'Common/Event.cpp',
        'Common/Object.cpp',
        'Common/String.cpp',
        'Node/Text/Font.cpp',
        'Node/Text/TextLayout.cpp',
        'Node/Text/TextStyle.cpp',
        'Tool/Music.cpp',
        'Tool/MusicPlayer.cpp'
    ],
    'Physics': [
        'Node/Shape/Shape.cpp',
        'Node/Shape/ShapeMaker.cpp'
    ],
    'Utils': [
        'Math/Matrix.cpp',
        'Math/Point.cpp',
        'Math/Rect.cpp',
        'Math/Size.cpp',
        'Tool/Data.cpp',
        'Tool/Path.cpp',
        'Tool/Random.cpp',
        'Tool/Timer.cpp'
    ]
};

function moveFile(oldPath, newPath) {
    const dir = path.dirname(newPath);
    if (!fs.existsSync(dir)) {
        fs.mkdirSync(dir, { recursive: true });
    }
    fs.renameSync(oldPath, newPath);
    console.log(`移动: ${oldPath} -> ${newPath}`);
}

function main() {
    console.log('开始重构Easy2D文件结构...\n');
    
    let movedCount = 0;
    let errorCount = 0;
    
    for (const [targetDir, files] of Object.entries(fileMappings)) {
        console.log(`处理目录: ${targetDir}`);
        
        for (const file of files) {
            const oldPath = path.join(srcPath, file);
            const newPath = path.join(srcPath, targetDir, path.basename(file));
            
            try {
                if (fs.existsSync(oldPath)) {
                    moveFile(oldPath, newPath);
                    movedCount++;
                } else {
                    console.log(`  跳过(不存在): ${oldPath}`);
                    errorCount++;
                }
            } catch (error) {
                console.error(`  错误: ${error.message}`);
                errorCount++;
            }
        }
        console.log('');
    }
    
    console.log(`\n重构完成!`);
    console.log(`成功移动: ${movedCount} 个文件`);
    console.log(`错误/跳过: ${errorCount} 个文件`);
}

main();
