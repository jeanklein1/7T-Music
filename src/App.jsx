import { useState, useEffect, Suspense, lazy, useCallback } from "react";
import TOOLS from "./tools/registry";

/* ═══════════════════════════════════════════════════════════════════════
   7T TOOL LAUNCHER
   
   Hash routing:  /#theme_tool  /#stamp  /#pawn_designer
   No hash  →  landing menu
   ═══════════════════════════════════════════════════════════════════════ */

/* ── cached lazy components ── */
const cache = {};
function getLazy(tool) {
  if (!cache[tool.id]) {
    cache[tool.id] = lazy(() => tool.load());
  }
  return cache[tool.id];
}

/* ── read / write hash ── */
function readHash() {
  return window.location.hash.replace(/^#\/?/, "") || null;
}

export default function App() {
  const [activeId, setActiveId] = useState(readHash);

  useEffect(() => {
    const onHash = () => setActiveId(readHash());
    window.addEventListener("hashchange", onHash);
    return () => window.removeEventListener("hashchange", onHash);
  }, []);

  const navigate = useCallback((id) => {
    window.location.hash = id ? `#${id}` : "";
    if (!id) setActiveId(null);        // clear forces menu
  }, []);

  const tool = TOOLS.find((t) => t.id === activeId);

  /* ── tool view ── */
  if (tool) {
    const Comp = getLazy(tool);
    return (
      <div style={S.root}>
        <nav style={S.nav}>
          <button style={S.back} onClick={() => navigate(null)}>
            ← menu
          </button>
          <span style={{ ...S.navTitle, color: tool.color }}>{tool.name}</span>
        </nav>
        <div style={S.toolWrap}>
          <Suspense fallback={<div style={S.loading}>loading…</div>}>
            <Comp />
          </Suspense>
        </div>
      </div>
    );
  }

  /* ── landing menu ── */
  return (
    <div style={S.root}>
      <header style={S.header}>
        <h1 style={S.title}>7T</h1>
        <p style={S.sub}>design tools</p>
      </header>
      <div style={S.grid}>
        {TOOLS.map((t) => (
          <button
            key={t.id}
            style={{ ...S.card, borderLeftColor: t.color }}
            onClick={() => navigate(t.id)}
            onMouseEnter={(e) => {
              e.currentTarget.style.background = "rgba(255,255,255,.06)";
              e.currentTarget.style.borderLeftColor = t.color;
            }}
            onMouseLeave={(e) => {
              e.currentTarget.style.background = "rgba(255,255,255,.025)";
              e.currentTarget.style.borderLeftColor = t.color;
            }}
          >
            <span style={{ ...S.cardDot, background: t.color }} />
            <span style={S.cardName}>{t.name}</span>
            <span style={S.cardDesc}>{t.desc}</span>
            <span style={S.cardId}>#{t.id}</span>
          </button>
        ))}
      </div>
      <footer style={S.footer}>
        drop a <code style={S.code}>7t_*.jsx</code> into <code style={S.code}>src/tools/</code> + register → done
      </footer>
    </div>
  );
}

/* ═══ styles ═══ */
const S = {
  root: {
    minHeight: "100vh",
    background: "#111",
    color: "#ccc",
    fontFamily: "'IBM Plex Mono', 'Consolas', monospace",
  },
  /* ── nav bar (inside a tool) ── */
  nav: {
    display: "flex", alignItems: "center", gap: 12,
    padding: "6px 14px",
    background: "#1a1a1a",
    borderBottom: "1px solid #2a2a2a",
    position: "sticky", top: 0, zIndex: 9999,
  },
  back: {
    background: "none", border: "1px solid #333", color: "#999",
    padding: "3px 10px", borderRadius: 3, cursor: "pointer",
    fontFamily: "inherit", fontSize: 12,
  },
  navTitle: {
    fontSize: 13, fontWeight: 600, letterSpacing: 0.5,
  },
  toolWrap: {
    /* let the tool fill the rest */
  },
  loading: {
    padding: 40, textAlign: "center", color: "#555", fontSize: 13,
  },
  /* ── landing ── */
  header: {
    padding: "60px 40px 20px",
  },
  title: {
    fontSize: 48, fontWeight: 700, color: "#eee", margin: 0,
    letterSpacing: 4,
  },
  sub: {
    fontSize: 14, color: "#666", margin: "6px 0 0", letterSpacing: 2,
    textTransform: "uppercase",
  },
  grid: {
    display: "grid",
    gridTemplateColumns: "repeat(auto-fill, minmax(320px, 1fr))",
    gap: 16, padding: "30px 40px",
  },
  card: {
    display: "flex", flexDirection: "column", gap: 8,
    padding: "20px 22px",
    background: "rgba(255,255,255,.025)",
    border: "none",
    borderLeft: "3px solid #555",
    borderRadius: 4,
    cursor: "pointer",
    textAlign: "left",
    fontFamily: "inherit",
    transition: "background .15s",
    position: "relative",
  },
  cardDot: {
    width: 8, height: 8, borderRadius: "50%",
    display: "inline-block", flexShrink: 0,
  },
  cardName: {
    fontSize: 16, fontWeight: 600, color: "#ddd",
  },
  cardDesc: {
    fontSize: 12, color: "#777", lineHeight: 1.5,
  },
  cardId: {
    fontSize: 11, color: "#444", fontFamily: "inherit",
    marginTop: 4,
  },
  footer: {
    padding: "30px 40px", fontSize: 12, color: "#444",
  },
  code: {
    background: "rgba(255,255,255,.06)", padding: "1px 6px",
    borderRadius: 3, fontSize: 12, color: "#888",
  },
};
