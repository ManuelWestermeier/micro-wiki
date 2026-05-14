const fs = require("fs");

function esc(txt) {
    return txt
        // Deutsche Zeichen
        .replace(/ä/g, "ae").replace(/ö/g, "oe").replace(/ü/g, "ue")
        .replace(/Ä/g, "Ae").replace(/Ö/g, "Oe").replace(/Ü/g, "Ue")
        .replace(/ß/g, "ss")

        // Akzente allgemein
        .replace(/[éèêë]/g, "e")
        .replace(/[áàâ]/g, "a")
        .replace(/[íìî]/g, "i")
        .replace(/[óòô]/g, "o")
        .replace(/[úùû]/g, "u")
        .replace(/ñ/g, "n")
        .replace(/ç/g, "c")

        // Hoch-/Tiefstellungen
        .replace(/²/g, "^2").replace(/³/g, "^3").replace(/¹/g, "^1")
        .replace(/⁰/g, "^0").replace(/⁴/g, "^4").replace(/⁵/g, "^5")
        .replace(/⁶/g, "^6").replace(/⁷/g, "^7").replace(/⁸/g, "^8").replace(/⁹/g, "^9")
        .replace(/⁺/g, "^+").replace(/⁻/g, "^-").replace(/ⁿ/g, "^n")

        .replace(/₀/g, "_0").replace(/₁/g, "_1").replace(/₂/g, "_2")
        .replace(/₃/g, "_3").replace(/₄/g, "_4").replace(/₅/g, "_5")
        .replace(/₆/g, "_6").replace(/₇/g, "_7").replace(/₈/g, "_8")
        .replace(/₉/g, "_9").replace(/₊/g, "_+").replace(/₋/g, "_-")
        .replace(/ₐ/g, "_a").replace(/ₙ/g, "_n").replace(/ₖ/g, "_k")

        // Griechisch
        .replace(/α/g, "alpha").replace(/β/g, "beta").replace(/γ/g, "gamma")
        .replace(/δ/g, "delta").replace(/ε/g, "epsilon").replace(/λ/g, "lambda")
        .replace(/μ/g, "mu").replace(/σ/g, "sigma").replace(/ρ/g, "rho")
        .replace(/ω/g, "omega").replace(/θ/g, "theta").replace(/φ/g, "phi")

        .replace(/Δ/g, "DELTA").replace(/Σ/g, "SUM").replace(/Ω/g, "OMEGA")
        .replace(/Φ/g, "PHI").replace(/Ψ/g, "PSI").replace(/Λ/g, "LAMBDA")

        // Operatoren
        .replace(/−/g, "-")
        .replace(/≈/g, "~")
        .replace(/≠/g, "!=")
        .replace(/≤/g, "<=")
        .replace(/≥/g, ">=")
        .replace(/≡/g, "===")
        .replace(/∝/g, "PROP")
        .replace(/∞/g, "INF")

        // Pfeile
        .replace(/→/g, "=>")
        .replace(/←/g, "<=")
        .replace(/↔/g, "<=>")
        .replace(/⇒/g, "=>")
        .replace(/⇔/g, "<=>")
        .replace(/↑/g, "^")
        .replace(/↓/g, "v")

        // Mengen
        .replace(/ℝ/g, "R").replace(/ℕ/g, "N").replace(/ℤ/g, "Z")
        .replace(/ℚ/g, "Q").replace(/ℂ/g, "C")

        // Sonstige
        .replace(/·/g, "*")
        .replace(/½/g, "1/2")
        .replace(/§/g, "PAR")
        .replace(/€/g, "EUR")
        .replace(/∫/g, "INT")
        .replace(/⃗/g, "->")
        .replace(/̄/g, "-")
        .replace(/∼/g, "~")

        // Typografie
        .replace(/–/g, "-")
        .replace(/—/g, "-")
        .replace(/…/g, "...")
        .replace(/[“”]/g, '"')
        .replace(/[‘’]/g, "'")

        // Fallback: alles nicht-ASCII entfernen
        .replace(/[^\x20-\x7E]/g, "");
}

fs.writeFileSync("wissen.json", JSON.stringify(JSON.parse(esc(fs.readFileSync("wissen.json", "utf-8"))), null, 2), "utf-8")