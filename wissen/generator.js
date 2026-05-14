const fs = require("fs");

const wissen = JSON.parse(fs.readFileSync("./wissen.json", "utf-8"));

const outDir = "../data";

function encodeArticle(articleValue) {
    let buf = [];

    for (const it of articleValue) {
        const type = it.typ;
        const typeId =
            type === "H" ? 0 :
                type === "T" ? 1 :
                    type === "LI" ? 2 : 0;

        const dataArr = Array.isArray(it.daten) ? it.daten : [it.daten];

        for (const d of dataArr) {
            const text = String(d ?? "");
            const textBuf = Buffer.from(text, "utf8");

            const b = Buffer.alloc(3);
            b.writeUInt8(typeId, 0);
            b.writeUInt16LE(textBuf.length, 1);

            buf.push(b, textBuf);
        }
    }

    return Buffer.concat(buf);
}

for (const [key, value] of Object.entries(wissen)) {
    for (const [subKey, subValue] of Object.entries(value)) {
        for (const [articleKey, articleValue] of Object.entries(subValue)) {

            const dir = `${outDir}/${key}/${subKey}`;
            fs.mkdirSync(dir, { recursive: true });

            const fileName = `${dir}/${articleKey}.bin`;

            const bin = encodeArticle(articleValue);

            fs.writeFileSync(fileName, bin);
        }
    }
}