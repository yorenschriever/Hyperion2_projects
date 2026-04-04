import {html, useState, useEffect, useRef, useCallback, render} from '../common/preact-standalone.js'

// ── Helpers ──────────────────────────────────────────────────────────────────

function sanitizeName(name) {
  let s = name.toLowerCase().replace(/[^a-z0-9_]/g, '_').replace(/^[0-9]+/, '');
  return s || 'palette';
}

function rgbToHex({ r, g, b }) {
  return '#' + [r, g, b].map(c => c.toString(16).padStart(2, '0')).join('');
}

function hexToRgb(hex) {
  const m = hex.match(/^#?([0-9a-f]{6})$/i);
  if (!m) return null;
  const v = parseInt(m[1], 16);
  return { r: (v >> 16) & 255, g: (v >> 8) & 255, b: v & 255 };
}

function rgbToHsl({ r, g, b }) {
  r /= 255; g /= 255; b /= 255;
  const max = Math.max(r, g, b), min = Math.min(r, g, b);
  let h2 = 0, s = 0, l = (max + min) / 2;
  if (max !== min) {
    const d = max - min;
    s = l > 0.5 ? d / (2 - max - min) : d / (max + min);
    if (max === r) h2 = ((g - b) / d + (g < b ? 6 : 0)) / 6;
    else if (max === g) h2 = ((b - r) / d + 2) / 6;
    else h2 = ((r - g) / d + 4) / 6;
  }
  return { h: Math.round(h2 * 360), s: Math.round(s * 100), l: Math.round(l * 100) };
}

function hslToRgb({ h: hue, s: sat, l: lit }) {
  const h2 = hue / 360, s = sat / 100, l = lit / 100;
  if (s === 0) { const v = Math.round(l * 255); return { r: v, g: v, b: v }; }
  const hue2rgb = (p, q, t) => {
    if (t < 0) t += 1; if (t > 1) t -= 1;
    if (t < 1/6) return p + (q - p) * 6 * t;
    if (t < 1/2) return q;
    if (t < 2/3) return p + (q - p) * (2/3 - t) * 6;
    return p;
  };
  const q = l < 0.5 ? l * (1 + s) : l + s - l * s;
  const p = 2 * l - q;
  return {
    r: Math.round(hue2rgb(p, q, h2 + 1/3) * 255),
    g: Math.round(hue2rgb(p, q, h2) * 255),
    b: Math.round(hue2rgb(p, q, h2 - 1/3) * 255),
  };
}

function clamp(v, lo, hi) { return Math.max(lo, Math.min(hi, v)); }

function gradientCSS(stops) {
  if (stops.length === 0) return 'linear-gradient(to right, #000, #000)';
  if (stops.length === 1) {
    const c = rgbToHex(stops[0].color);
    return `linear-gradient(to right, ${c}, ${c})`;
  }
  const sorted = [...stops].sort((a, b) => a.position - b.position);
  const parts = sorted.map(s => `${rgbToHex(s.color)} ${(s.position / 255 * 100).toFixed(1)}%`);
  return `linear-gradient(to right, ${parts.join(', ')})`;
}

// ── AI stub ──────────────────────────────────────────────────────────────────

async function generatePaletteFromPrompt(prompt) {
  // TODO: replace with real API call
  return {
    name: "Generated Palette",
    primary: { r: 255, g: 94, b: 77 },
    secondary: { r: 45, g: 0, b: 80 },
    highlight: { r: 255, g: 214, b: 0 },
    gradient: [
      { position: 0, color: { r: 45, g: 0, b: 80 } },
      { position: 128, color: { r: 255, g: 94, b: 77 } },
      { position: 255, color: { r: 255, g: 214, b: 0 } },
    ],
  };
}

// ── Import parser ────────────────────────────────────────────────────────────

function parseHpp(text) {
  const rgbRe = /RGB\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\)/g;
  const posRe = /\.position\s*=\s*(\d+)\s*,\s*\.color\s*=\s*RGB\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\)/g;

  const gradient = [];
  let m;
  while ((m = posRe.exec(text)) !== null) {
    gradient.push({ position: +m[1], color: { r: +m[2], g: +m[3], b: +m[4] } });
  }

  const findField = (field) => {
    const re = new RegExp(`\\.${field}\\s*=\\s*RGB\\(\\s*(\\d+)\\s*,\\s*(\\d+)\\s*,\\s*(\\d+)\\s*\\)`);
    const fm = text.match(re);
    return fm ? { r: +fm[1], g: +fm[2], b: +fm[3] } : { r: 128, g: 128, b: 128 };
  };

  const nameMatch = text.match(/\.name\s*=\s*"([^"]*)"/);
  const name = nameMatch ? nameMatch[1] : 'Imported';

  return {
    name,
    primary: findField('primary'),
    secondary: findField('secondary'),
    highlight: findField('highlight'),
    gradient,
  };
}

// ── Export ────────────────────────────────────────────────────────────────────

function exportHpp(palette) {
  const sn = sanitizeName(palette.name);
  const sorted = [...palette.gradient].sort((a, b) => a.position - b.position);
  const stops = sorted.map(s =>
    `        {.position = ${s.position}, .color = RGB(${s.color.r}, ${s.color.g}, ${s.color.b})}`
  ).join(',\n');

  return `Palette ${sn}{
    .gradient = Gradient({
${stops}
    }),
    .primary = RGB(${palette.primary.r}, ${palette.primary.g}, ${palette.primary.b}),
    .secondary = RGB(${palette.secondary.r}, ${palette.secondary.g}, ${palette.secondary.b}),
    .highlight = RGB(${palette.highlight.r}, ${palette.highlight.g}, ${palette.highlight.b}),
    .name = "${palette.name}"};
`;
}

function downloadFile(filename, content) {
  const blob = new Blob([content], { type: 'text/plain' });
  const a = document.createElement('a');
  a.href = URL.createObjectURL(blob);
  a.download = filename;
  a.click();
  URL.revokeObjectURL(a.href);
}

// ── LocalStorage ─────────────────────────────────────────────────────────────

const STORAGE_KEY = 'palette-editor-palettes';

function getSavedPalettes() {
  try {
    return JSON.parse(localStorage.getItem(STORAGE_KEY)) || {};
  } catch { return {}; }
}

function savePaletteToStorage(palette) {
  const all = getSavedPalettes();
  all[palette.name] = palette;
  localStorage.setItem(STORAGE_KEY, JSON.stringify(all));
}

function removePaletteFromStorage(name) {
  const all = getSavedPalettes();
  delete all[name];
  localStorage.setItem(STORAGE_KEY, JSON.stringify(all));
}

// ── Components ───────────────────────────────────────────────────────────────

function ColorPicker({ color, onChange }) {
  const hsl = rgbToHsl(color);
  const hex = rgbToHex(color);

  const setHsl = (field, val) => {
    const next = { ...hsl, [field]: val };
    onChange(hslToRgb(next));
  };

  const onHex = (e) => {
    const rgb = hexToRgb(e.target.value);
    if (rgb) onChange(rgb);
  };

  return html`
    <div class="swatch-large" style=${{ background: hex }}></div>
    <div class="hsl-sliders">
      <div class="slider-row">
        <span>H</span>
        <input type="range" min="0" max="360" value=${hsl.h}
          onInput=${e => setHsl('h', +e.target.value)}
          style=${{ background: `linear-gradient(to right, hsl(0,100%,50%),hsl(60,100%,50%),hsl(120,100%,50%),hsl(180,100%,50%),hsl(240,100%,50%),hsl(300,100%,50%),hsl(360,100%,50%))` }} />
        <input type="number" min="0" max="360" value=${hsl.h}
          onInput=${e => setHsl('h', clamp(+e.target.value, 0, 360))} />
      </div>
      <div class="slider-row">
        <span>S</span>
        <input type="range" min="0" max="100" value=${hsl.s}
          onInput=${e => setHsl('s', +e.target.value)} />
        <input type="number" min="0" max="100" value=${hsl.s}
          onInput=${e => setHsl('s', clamp(+e.target.value, 0, 100))} />
      </div>
      <div class="slider-row">
        <span>L</span>
        <input type="range" min="0" max="100" value=${hsl.l}
          onInput=${e => setHsl('l', +e.target.value)} />
        <input type="number" min="0" max="100" value=${hsl.l}
          onInput=${e => setHsl('l', clamp(+e.target.value, 0, 100))} />
      </div>
    </div>
    <div class="hex-row">
      <span>#</span>
      <input type="text" value=${hex} maxlength="7" onInput=${onHex} />
    </div>
  `;
}

function GradientEditor({ stops, onChange, selectedIdx, onSelect }) {
  const trackRef = useRef(null);
  const dragging = useRef(null);

  const sorted = [...stops].sort((a, b) => a.position - b.position);

  const updateStop = (idx, patch) => {
    const next = stops.map((s, i) => i === idx ? { ...s, ...patch } : s);
    onChange(next);
  };

  const addStop = () => {
    if (stops.length === 0) {
      onChange([{ position: 128, color: { r: 128, g: 128, b: 128 } }]);
      return;
    }
    // insert at midpoint of largest gap
    const s = [...stops].sort((a, b) => a.position - b.position);
    let bestGap = -1, insertPos = 128;
    for (let i = 0; i < s.length - 1; i++) {
      const gap = s[i + 1].position - s[i].position;
      if (gap > bestGap) { bestGap = gap; insertPos = Math.round((s[i].position + s[i + 1].position) / 2); }
    }
    // also consider 0..first and last..255
    if (s[0].position > bestGap) { bestGap = s[0].position; insertPos = Math.round(s[0].position / 2); }
    if (255 - s[s.length - 1].position > bestGap) { insertPos = Math.round((s[s.length - 1].position + 255) / 2); }
    onChange([...stops, { position: insertPos, color: { r: 128, g: 128, b: 128 } }]);
    onSelect(stops.length);
  };

  const removeStop = () => {
    if (selectedIdx == null || selectedIdx >= stops.length) return;
    const next = stops.filter((_, i) => i !== selectedIdx);
    onChange(next);
    onSelect(null);
  };

  // Drag handling
  const onPointerDown = (e, idx) => {
    e.preventDefault();
    dragging.current = idx;
    onSelect(idx);
    document.addEventListener('pointermove', onPointerMove);
    document.addEventListener('pointerup', onPointerUp);
  };

  const onPointerMove = useCallback((e) => {
    if (dragging.current == null || !trackRef.current) return;
    const rect = trackRef.current.getBoundingClientRect();
    const x = clamp((e.clientX - rect.left) / rect.width, 0, 1);
    const pos = Math.round(x * 255);
    const next = stops.map((s, i) => i === dragging.current ? { ...s, position: pos } : s);
    onChange(next);
  }, [stops, onChange]);

  const onPointerUp = useCallback(() => {
    dragging.current = null;
    document.removeEventListener('pointermove', onPointerMove);
    document.removeEventListener('pointerup', onPointerUp);
  }, [onPointerMove]);

  // Click on gradient bar to add a stop
  const onBarClick = (e) => {
    const rect = e.currentTarget.getBoundingClientRect();
    const x = clamp((e.clientX - rect.left) / rect.width, 0, 1);
    const pos = Math.round(x * 255);
    onChange([...stops, { position: pos, color: { r: 128, g: 128, b: 128 } }]);
    onSelect(stops.length);
  };

  return html`
    <div class="gradient-bar" style=${{ background: gradientCSS(stops) }} onClick=${onBarClick}></div>
    <div class="stops-track" ref=${trackRef}>
      ${stops.map((s, i) => html`
        <div class=${"stop-handle" + (i === selectedIdx ? " selected" : "")}
          style=${{ left: `${(s.position / 255) * 100}%`, background: rgbToHex(s.color) }}
          onPointerDown=${(e) => onPointerDown(e, i)} />
      `)}
    </div>
    <div class="stop-buttons">
      <button class="small" onClick=${addStop}>+ Add stop</button>
      <button class="small secondary" onClick=${removeStop} disabled=${selectedIdx == null}>− Remove</button>
    </div>
    <div class="stop-list">
      ${stops.map((s, i) => html`
        <div class=${"stop-row" + (i === selectedIdx ? " selected" : "")} onClick=${() => onSelect(i)}>
          <div class="swatch" style=${{ background: rgbToHex(s.color) }}></div>
          <input type="number" min="0" max="255" value=${s.position}
            onInput=${e => updateStop(i, { position: clamp(+e.target.value, 0, 255) })} />
          <input type="text" value=${rgbToHex(s.color)} maxlength="7"
            onInput=${e => { const rgb = hexToRgb(e.target.value); if (rgb) updateStop(i, { color: rgb }); }} />
        </div>
      `)}
    </div>
  `;
}

// ── App ──────────────────────────────────────────────────────────────────────

function App() {
  const [name, setName] = useState('My Palette');
  const [primary, setPrimary] = useState({ r: 255, g: 0, b: 0 });
  const [secondary, setSecondary] = useState({ r: 0, g: 128, b: 255 });
  const [highlight, setHighlight] = useState({ r: 255, g: 255, b: 255 });
  const [gradient, setGradient] = useState([
    { position: 0, color: { r: 0, g: 0, b: 0 } },
    { position: 128, color: { r: 255, g: 0, b: 0 } },
    { position: 255, color: { r: 255, g: 255, b: 255 } },
  ]);
  const [selectedStop, setSelectedStop] = useState(null);
  const [prompt, setPrompt] = useState('');
  const [importText, setImportText] = useState('');
  const [savedNames, setSavedNames] = useState(() => Object.keys(getSavedPalettes()));

  const loadPalette = (p) => {
    setName(p.name);
    setPrimary(p.primary);
    setSecondary(p.secondary);
    setHighlight(p.highlight);
    setGradient(p.gradient);
    setSelectedStop(null);
  };

  const onGenerate = async () => {
    const p = await generatePaletteFromPrompt(prompt);
    loadPalette(p);
  };

  const onParse = () => {
    try {
      const p = parseHpp(importText);
      loadPalette(p);
    } catch (e) {
      alert('Parse error: ' + e.message);
    }
  };

  const onExport = () => {
    const content = exportHpp({ name, primary, secondary, highlight, gradient });
    downloadFile(sanitizeName(name) + '.hpp', content);
  };

  const onSave = () => {
    savePaletteToStorage({ name, primary, secondary, highlight, gradient });
    setSavedNames(Object.keys(getSavedPalettes()));
  };

  const onLoad = (paletteName) => {
    const all = getSavedPalettes();
    if (all[paletteName]) loadPalette(all[paletteName]);
  };

  const onRemove = (paletteName) => {
    removePaletteFromStorage(paletteName);
    setSavedNames(Object.keys(getSavedPalettes()));
  };

  // Update selected stop color via the color picker shown below gradient
  const selectedStopColor = selectedStop != null && gradient[selectedStop]
    ? gradient[selectedStop].color : null;

  const onSelectedStopColorChange = (rgb) => {
    if (selectedStop == null) return;
    setGradient(gradient.map((s, i) => i === selectedStop ? { ...s, color: rgb } : s));
  };

  return html`
    <h1>Palette Editor</h1>

    <!-- Preview -->
    <div class="section">
      <h2>Preview</h2>
      <div class="preview-row">
        <div class="preview-gradient" style=${{ background: gradientCSS(gradient) }}></div>
        <div class="preview-swatches">
          <div class="preview-swatch" style=${{ background: rgbToHex(primary) }}>pri</div>
          <div class="preview-swatch" style=${{ background: rgbToHex(secondary) }}>sec</div>
          <div class="preview-swatch" style=${{ background: rgbToHex(highlight) }}>hi</div>
        </div>
      </div>
    </div>

    <!-- Primary / Secondary / Highlight -->
    <div class="section">
      <h2>Colors</h2>
      <div class="color-fields">
        <div class="color-field">
          <label>Primary</label>
          <${ColorPicker} color=${primary} onChange=${setPrimary} />
        </div>
        <div class="color-field">
          <label>Secondary</label>
          <${ColorPicker} color=${secondary} onChange=${setSecondary} />
        </div>
        <div class="color-field">
          <label>Highlight</label>
          <${ColorPicker} color=${highlight} onChange=${setHighlight} />
        </div>
      </div>
    </div>

    <!-- Gradient -->
    <div class="section">
      <h2>Gradient</h2>
      <${GradientEditor}
        stops=${gradient}
        onChange=${setGradient}
        selectedIdx=${selectedStop}
        onSelect=${setSelectedStop} />
      ${selectedStopColor && html`
        <div class="mt-12">
          <h2>Stop Color</h2>
          <${ColorPicker} color=${selectedStopColor} onChange=${onSelectedStopColorChange} />
        </div>
      `}
    </div>

    <!-- Name -->
    <div class="section">
      <h2>Name</h2>
      <div class="name-row">
        <input type="text" value=${name} onInput=${e => setName(e.target.value)} placeholder="Palette name" />
        <span class="sanitized">${sanitizeName(name)}</span>
      </div>
    </div>

    <!-- Save / Export -->
    <div class="section">
      <h2>Save / Export</h2>
      <div class="export-section">
        <button onClick=${onSave}>Save to browser</button>
        <button onClick=${onExport}>Download .hpp</button>
      </div>
      ${savedNames.length > 0 && html`
        <div class="mt-12">
          <h2>Saved palettes</h2>
          <div class="saved-list">
            ${savedNames.map(n => html`
              <div class="saved-row">
                <span class="saved-name" onClick=${() => onLoad(n)}>${n}</span>
                <button class="small" onClick=${() => onLoad(n)}>Load</button>
                <button class="small secondary" onClick=${() => onRemove(n)}>Remove</button>
              </div>
            `)}
          </div>
        </div>
      `}
    </div>

    <!-- AI Prompt -->
    <div class="section">
      <h2>AI Generate</h2>
      <div class="prompt-row">
        <input type="text" value=${prompt} onInput=${e => setPrompt(e.target.value)}
          placeholder="Describe a palette..." onKeyDown=${e => e.key === 'Enter' && onGenerate()} />
        <button onClick=${onGenerate}>Generate</button>
      </div>
    </div>

    <!-- Import -->
    <div class="section import-section">
      <h2>Import</h2>
      <textarea value=${importText} onInput=${e => setImportText(e.target.value)}
        placeholder="Paste .hpp palette definition here..." />
      <button onClick=${onParse}>Parse</button>
    </div>

  `;
}

render(html`<${App} />`, document.getElementById('app'));
