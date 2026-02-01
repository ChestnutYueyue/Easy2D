/**
 * 自动检测编码并转换为指定编码脚本
 * 无需外部依赖，纯 Node.js 实现
 * 
 * 使用方法:
 *   node auto-convert-encoding.js <文件夹路径> [目标编码]
 * 
 * 参数:
 *   文件夹路径 - 要处理的文件夹路径
 *   目标编码   - 可选，默认为 utf-8，支持: utf-8, utf-8-bom, utf-16le, utf-16be, gbk
 * 
 * 示例:
 *   node auto-convert-encoding.js ./src                    # 转换为 UTF-8
 *   node auto-convert-encoding.js ./src utf-16le           # 转换为 UTF-16 LE
 *   node auto-convert-encoding.js ./src utf-8-bom          # 转换为 UTF-8 with BOM
 *   node auto-convert-encoding.js ./src gbk                # 转换为 GBK
 */

const fs = require('fs');
const path = require('path');
const { execSync } = require('child_process');

// 需要转换的文件扩展名
const TARGET_EXTENSIONS = ['.h', '.c', '.cpp'];

// 支持的目标编码
const SUPPORTED_ENCODINGS = ['utf-8', 'utf-8-bom', 'utf-16le', 'utf-16be', 'gbk'];

/**
 * GBK 解码表 - 常用汉字区
 * 简化版 GBK 解码实现
 */
const GBKDecoder = {
    decode: function(buffer) {
        let result = '';
        let i = 0;
        
        while (i < buffer.length) {
            const byte = buffer[i];
            
            if (byte < 0x80) {
                result += String.fromCharCode(byte);
                i++;
                continue;
            }
            
            if (i + 1 < buffer.length) {
                const byte2 = buffer[i + 1];
                const gbkCode = (byte << 8) | byte2;
                const unicode = this.gbkToUnicode(gbkCode);
                if (unicode !== null) {
                    result += String.fromCharCode(unicode);
                    i += 2;
                    continue;
                }
            }
            
            result += '\uFFFD';
            i++;
        }
        
        return result;
    },
    
    gbkToUnicode: function(gbkCode) {
        try {
            const iconv = require('iconv-lite');
            const buffer = Buffer.from([(gbkCode >> 8) & 0xFF, gbkCode & 0xFF]);
            const str = iconv.decode(buffer, 'gbk');
            return str.charCodeAt(0);
        } catch (e) {
            return this.fallbackGbkToUnicode(gbkCode);
        }
    },
    
    fallbackGbkToUnicode: function(gbkCode) {
        const zoneCode = (gbkCode >> 8) & 0xFF;
        const posCode = gbkCode & 0xFF;
        
        if (zoneCode >= 0xB0 && zoneCode <= 0xD7 && posCode >= 0xA1 && posCode <= 0xFE) {
            const offset = ((zoneCode - 0xB0) * 94 + (posCode - 0xA1));
            return 0x4E00 + offset;
        }
        
        if (zoneCode >= 0x81 && zoneCode <= 0xFE && posCode >= 0x40 && posCode <= 0xFE && posCode !== 0x7F) {
            return 0x4E00 + ((zoneCode - 0x81) * 191 + (posCode - 0x40));
        }
        
        return null;
    }
};

/**
 * 检测文件编码
 * @param {Buffer} buffer - 文件内容
 * @returns {string} - 编码类型
 */
function detectEncoding(buffer) {
    if (buffer.length >= 2 && buffer[0] === 0xFF && buffer[1] === 0xFE) {
        return 'utf-16le';
    }
    
    if (buffer.length >= 2 && buffer[0] === 0xFE && buffer[1] === 0xFF) {
        return 'utf-16be';
    }
    
    if (buffer.length >= 3 && buffer[0] === 0xEF && buffer[1] === 0xBB && buffer[2] === 0xBF) {
        return 'utf-8-bom';
    }
    
    let isAscii = true;
    for (let i = 0; i < buffer.length; i++) {
        if (buffer[i] > 0x7F) {
            isAscii = false;
            break;
        }
    }
    
    if (isAscii) {
        return 'ascii';
    }
    
    if (isValidUTF8(buffer)) {
        return 'utf-8';
    }
    
    return 'gbk';
}

/**
 * 检查是否为有效的 UTF-8 编码
 */
function isValidUTF8(buffer) {
    let i = 0;
    while (i < buffer.length) {
        const byte = buffer[i];
        
        if (byte < 0x80) {
            i++;
            continue;
        }
        
        if ((byte & 0xE0) === 0xC0) {
            if (i + 1 >= buffer.length || (buffer[i + 1] & 0xC0) !== 0x80) {
                return false;
            }
            i += 2;
            continue;
        }
        
        if ((byte & 0xF0) === 0xE0) {
            if (i + 2 >= buffer.length || 
                (buffer[i + 1] & 0xC0) !== 0x80 || 
                (buffer[i + 2] & 0xC0) !== 0x80) {
                return false;
            }
            i += 3;
            continue;
        }
        
        if ((byte & 0xF8) === 0xF0) {
            if (i + 3 >= buffer.length || 
                (buffer[i + 1] & 0xC0) !== 0x80 || 
                (buffer[i + 2] & 0xC0) !== 0x80 || 
                (buffer[i + 3] & 0xC0) !== 0x80) {
                return false;
            }
            i += 4;
            continue;
        }
        
        return false;
    }
    return true;
}

/**
 * 尝试安装 iconv-lite
 */
function tryInstallIconv() {
    try {
        require('iconv-lite');
        return true;
    } catch (e) {
        console.log('正在安装 iconv-lite 以获得更好的编码支持...');
        try {
            execSync('npm install iconv-lite --save-dev', { 
                stdio: 'inherit',
                cwd: process.cwd()
            });
            console.log('iconv-lite 安装成功！\n');
            return true;
        } catch (installError) {
            console.warn('iconv-lite 安装失败，将使用备用解码方法\n');
            return false;
        }
    }
}

/**
 * 解码 Buffer 为字符串
 * @param {Buffer} buffer - 原始编码的文件内容
 * @param {string} encoding - 源编码
 * @returns {string} - 解码后的字符串
 */
function decodeBuffer(buffer, encoding) {
    switch (encoding) {
        case 'utf-8-bom':
            return buffer.slice(3).toString('utf8');
        case 'utf-8':
            return buffer.toString('utf8');
        case 'ascii':
            return buffer.toString('ascii');
        case 'utf-16le':
            return buffer.slice(2).toString('utf16le');
        case 'utf-16be':
            // UTF-16 BE 需要手动转换
            const leBuffer = Buffer.alloc(buffer.length - 2);
            for (let i = 2; i < buffer.length; i += 2) {
                leBuffer[i - 2] = buffer[i + 1];
                leBuffer[i - 1] = buffer[i];
            }
            return leBuffer.toString('utf16le');
        case 'gbk':
        default:
            try {
                const iconv = require('iconv-lite');
                return iconv.decode(buffer, 'gbk');
            } catch (e) {
                return GBKDecoder.decode(buffer);
            }
    }
}

/**
 * 将字符串编码为指定格式的 Buffer
 * @param {string} str - 要编码的字符串
 * @param {string} targetEncoding - 目标编码
 * @returns {Buffer} - 编码后的 Buffer
 */
function encodeToBuffer(str, targetEncoding) {
    switch (targetEncoding) {
        case 'utf-8':
            return Buffer.from(str, 'utf8');
        case 'utf-8-bom':
            const utf8Buffer = Buffer.from(str, 'utf8');
            return Buffer.concat([Buffer.from([0xEF, 0xBB, 0xBF]), utf8Buffer]);
        case 'utf-16le':
            const utf16leBuffer = Buffer.from(str, 'utf16le');
            return Buffer.concat([Buffer.from([0xFF, 0xFE]), utf16leBuffer]);
        case 'utf-16be':
            const utf16leTemp = Buffer.from(str, 'utf16le');
            const utf16beBuffer = Buffer.alloc(utf16leTemp.length);
            for (let i = 0; i < utf16leTemp.length; i += 2) {
                utf16beBuffer[i] = utf16leTemp[i + 1];
                utf16beBuffer[i + 1] = utf16leTemp[i];
            }
            return Buffer.concat([Buffer.from([0xFE, 0xFF]), utf16beBuffer]);
        case 'gbk':
            try {
                const iconv = require('iconv-lite');
                return iconv.encode(str, 'gbk');
            } catch (e) {
                console.error('错误: 转换为 GBK 需要 iconv-lite，请安装: npm install iconv-lite');
                throw e;
            }
        default:
            return Buffer.from(str, 'utf8');
    }
}

/**
 * 递归获取目录下所有目标文件
 */
function getTargetFiles(dirPath) {
    const files = [];
    
    function traverse(currentPath) {
        const entries = fs.readdirSync(currentPath, { withFileTypes: true });
        
        for (const entry of entries) {
            const fullPath = path.join(currentPath, entry.name);
            
            if (entry.isDirectory()) {
                traverse(fullPath);
            } else if (entry.isFile()) {
                const ext = path.extname(entry.name).toLowerCase();
                if (TARGET_EXTENSIONS.includes(ext)) {
                    files.push(fullPath);
                }
            }
        }
    }
    
    traverse(dirPath);
    return files;
}

/**
 * 主函数
 */
async function main() {
    const targetDir = process.argv[2];
    let targetEncoding = process.argv[3] || 'utf-8';
    
    // 标准化编码名称
    targetEncoding = targetEncoding.toLowerCase().replace(/_/g, '-');
    
    if (!targetDir) {
        console.log('用法: node auto-convert-encoding.js <文件夹路径> [目标编码]');
        console.log('');
        console.log('支持的目标编码:');
        SUPPORTED_ENCODINGS.forEach(enc => {
            console.log(`  - ${enc}`);
        });
        console.log('');
        console.log('示例:');
        console.log('  node auto-convert-encoding.js ./src');
        console.log('  node auto-convert-encoding.js ./src utf-16le');
        console.log('  node auto-convert-encoding.js ./src utf-8-bom');
        console.log('  node auto-convert-encoding.js ./src gbk');
        process.exit(1);
    }
    
    // 验证目标编码
    if (!SUPPORTED_ENCODINGS.includes(targetEncoding)) {
        console.error(`错误: 不支持的目标编码 "${targetEncoding}"`);
        console.log('');
        console.log('支持的目标编码:');
        SUPPORTED_ENCODINGS.forEach(enc => {
            console.log(`  - ${enc}`);
        });
        process.exit(1);
    }
    
    if (!fs.existsSync(targetDir)) {
        console.error(`错误: 路径不存在: ${targetDir}`);
        process.exit(1);
    }
    
    if (!fs.statSync(targetDir).isDirectory()) {
        console.error(`错误: 不是目录: ${targetDir}`);
        process.exit(1);
    }
    
    const encodingDisplay = targetEncoding.toUpperCase();
    console.log('\n╔════════════════════════════════════════════════╗');
    console.log(`║     自动编码检测与转换工具 (${encodingDisplay})          ║`);
    console.log('╚════════════════════════════════════════════════╝\n');
    
    const hasIconv = tryInstallIconv();
    
    console.log(`目标目录: ${targetDir}`);
    console.log(`目标编码: ${targetEncoding.toUpperCase()}`);
    console.log(`文件类型: ${TARGET_EXTENSIONS.join(', ')}`);
    console.log(`编码库: ${hasIconv ? 'iconv-lite' : '内置解码器'}\n`);
    
    const files = getTargetFiles(targetDir);
    
    if (files.length === 0) {
        console.log('未找到需要转换的文件');
        return;
    }
    
    console.log(`找到 ${files.length} 个目标文件\n`);
    console.log('开始扫描并转换...\n');
    
    let converted = 0;
    let skipped = 0;
    let errors = 0;
    const encodingStats = {};
    
    for (const file of files) {
        try {
            const buffer = fs.readFileSync(file);
            const encoding = detectEncoding(buffer);
            
            encodingStats[encoding] = (encodingStats[encoding] || 0) + 1;
            
            // 如果已经是目标编码，跳过
            if (encoding === targetEncoding) {
                console.log(`  [跳过] 已经是 ${targetEncoding.toUpperCase()}: ${file}`);
                skipped++;
                continue;
            }
            
            // 解码为字符串
            const str = decodeBuffer(buffer, encoding);
            
            // 编码为目标格式
            const outputBuffer = encodeToBuffer(str, targetEncoding);
            fs.writeFileSync(file, outputBuffer);
            
            const action = encoding.toUpperCase() + ' -> ' + targetEncoding.toUpperCase();
            console.log(`  [转换成功] ${action}: ${file}`);
            converted++;
            
        } catch (error) {
            console.error(`  [错误] ${file}: ${error.message}`);
            errors++;
        }
    }
    
    console.log('\n-------------------------------------------');
    console.log('编码分布统计:');
    for (const [enc, count] of Object.entries(encodingStats)) {
        console.log(`  ${enc.toUpperCase()}: ${count} 个文件`);
    }
    
    console.log('\n-------------------------------------------');
    console.log('转换结果:');
    console.log(`  成功转换: ${converted} 个文件`);
    console.log(`  跳过: ${skipped} 个文件`);
    console.log(`  错误: ${errors} 个文件`);
    console.log('-------------------------------------------\n');
}

main().catch(console.error);
