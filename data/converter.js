const fs = require("fs");

function normalizeU8g2(str) {
    let out = "";

    for (const ch of str) {
        const code = ch.codePointAt(0);

        if (code < 0x20 || code > 0x7E) continue;

        if (ch === "\\") {
            out += "/";
            continue;
        }

        out += ch;
    }

    return out;
}

function logCharDistribution(str) {
    const map = new Map();

    for (const ch of str) {
        const code = ch.codePointAt(0);
        const key = `${ch} (U+${code.toString(16).toUpperCase().padStart(4, "0")})`;
        map.set(key, (map.get(key) || 0) + 1);
    }

    const sorted = [...map.entries()].sort((a, b) => b[1] - a[1]);
    for (const [k, v] of sorted) {
        console.log(`${k}: ${v}`);
    }
}

const input = fs.readFileSync("wissen.json", "utf-8");

logCharDistribution(input);

const cleaned = normalizeU8g2(input);

logCharDistribution(cleaned);

fs.writeFileSync(
    "wissen.json",
    JSON.stringify(JSON.parse(cleaned), null, 2),
    "utf-8"
);