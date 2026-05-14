const fs = require('fs');

const wissen = JSON.parse(fs.readFileSync('./wissen.json', 'utf-8'));

const outDir = "../data"

for (const [key, value] of Object.entries(wissen)) {
    for (const [subKey, subValue] of Object.entries(value)) {
        for (const [articleKey, articleValue] of Object.entries(subValue)) {
            const fileName = `${outDir}/${key}/${subKey}/${articleKey}.json`;
            fs.mkdirSync(`${outDir}/${key}/${subKey}/`, { recursive: true });
            fs.writeFileSync(fileName, JSON.stringify(articleValue, null, 2));
        }
    }
}