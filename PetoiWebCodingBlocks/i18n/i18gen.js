const fs = require('fs');
const path = require('path'); 

const langs = ['zh', 'en', 'ja'];

function generateTSV() {
    
    // 1. 读取 translations.js
    const filePath = path.join(__dirname, '..', 'lang', 'translations.js');
    const content = fs.readFileSync(filePath, 'utf8');

    // 2. 提取 TRANSLATIONS 对象
    const match = content.match(/const TRANSLATIONS\s*=\s*({[\s\S]*?});\s*let currentLang/);
    if (!match) {
        console.error('TRANSLATIONS object not found!');
        process.exit(1);
    }
    const translations = eval('(' + match[1] + ')');

    // 3. 获取所有语言的键值
    const allKeys = new Set();
    // 确保所有语言对象中的所有键都被收集
    Object.values(translations).forEach(langObj => {
        if (langObj) {
            console.log(langObj);
            Object.keys(langObj).forEach(key => allKeys.add(key));
        }
    });

    // 4. 生成 TSV
    let tsv = 'key\t';
    for (const lang of langs) {
        tsv += `${lang}\t`;
    }
    tsv += '\n';

    for (const key of allKeys) {
        tsv += `${key}\t`;
        for (const lang of langs) {
            const value = translations[lang] && translations[lang][key] ? translations[lang][key] : '';
            tsv += `${value}\t`;
        }
        tsv += '\n';
    }

    // 5. 写入文件
    fs.writeFileSync('i18n.tsv', tsv, 'utf8');
    console.log('TSV file generated: translations.tsv');
}

function generateJS() {
    // 1. 读取TSV文件
    const tsvPath = path.join(__dirname, 'i18n.tsv');
    const tsvContent = fs.readFileSync(tsvPath, 'utf8');

    // 2. 解析TSV内容
    const lines = tsvContent.split('\n').filter(line => line.trim());
    const headers = lines[0].split('\t');
    
    // 动态初始化translations对象，包含所有支持的语言
    const translations = {};
    langs.forEach(lang => {
        translations[lang] = {};
    });

    // 3. 处理每一行数据
    for (let i = 1; i < lines.length; i++) {
        const values = lines[i].split('\t');
        const key = values[0];
        
        if (key) {
            // 为每个语言分配对应的值
            for (let j = 0; j < langs.length; j++) {
                const lang = langs[j];
                const value = values[j + 1] || ''; // j+1 因为第一列是key
                translations[lang][key] = value;
            }
        }
    }

    // 4. 读取原始的translations.js文件
    const jsPath = path.join(__dirname, '..', 'lang', 'translations.js');
    const jsContent = fs.readFileSync(jsPath, 'utf8');

    // 5. 生成新的translations.js内容
    const newContent = `const TRANSLATIONS = ${JSON.stringify(translations, null, 2)};\n\n` +
        jsContent.substring(jsContent.indexOf('let currentLang'));

    // 6. 写入更新后的文件
    fs.writeFileSync(jsPath, newContent, 'utf8');
    console.log('i18n.js 已更新完成！'); 
}


// 运行命令：node i18gen.js -t 生成TSV文件
// 运行命令：node i18gen.js -j 更新i18n.js文件

const args = process.argv.slice(2);
if (args.includes('-t')) {
    generateTSV();
}
if (args.includes('-j')) {
    generateJS();
}

