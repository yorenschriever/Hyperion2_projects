import { html, useState, useEffect, useRef} from './common/preact-standalone.js'
import { main as initWebGLMonitor } from './monitor/webgl-monitor.js';
import { pixelMap as defaultPixelMap } from './pixelMap.js';
import wasmFactory from './pattern.mjs';

const PARAM_NAMES = ['Velocity', 'Amount', 'Size', 'Variant', 'Offset', 'Intensity'];

const ParamSliders = ({ instance }) => {
    const [values, setValues] = useState([0.5, 0.5, 0.5, 0.5, 0.5, 0.5]);

    const onChange = (index, val) => {
        const next = [...values];
        next[index] = val;
        setValues(next);
        if (instance) instance._setParam(index, val);
    };

    return html`
        <div class="param-sliders">
            ${PARAM_NAMES.map((name, i) => html`
                <div class="param-row" key=${name}>
                    <label>${name}</label>
                    <input type="range" min="0" max="1" step="0.01" value=${values[i]}
                        onInput=${e => onChange(i, parseFloat(e.target.value))} />
                    <span>${values[i].toFixed(2)}</span>
                </div>
            `)}
        </div>
    `;
};

const PALETTE_STORAGE_KEY = 'palette-editor-palettes';

function getSavedPalettes() {
    try {
        return JSON.parse(localStorage.getItem(PALETTE_STORAGE_KEY)) || {};
    } catch { return {}; }
}

function applyPaletteToInstance(instance, palette) {
    if (!instance || !palette) return;
    const { primary, secondary, highlight, gradient } = palette;
    if (primary)   instance._setPrimary(primary.r, primary.g, primary.b);
    if (secondary) instance._setSecondary(secondary.r, secondary.g, secondary.b);
    if (highlight) instance._setHighlight(highlight.r, highlight.g, highlight.b);
    if (gradient && gradient.length > 0) {
        const sorted = [...gradient].sort((a, b) => a.position - b.position);
        sorted.forEach((stop, i) => {
            instance._setGradientStop(i, stop.position, stop.color.r, stop.color.g, stop.color.b);
        });
        instance._applyPalette(sorted.length);
    }
}

function rgbToHex({ r, g, b }) {
    return '#' + [r, g, b].map(c => c.toString(16).padStart(2, '0')).join('');
}

function gradientCSS(stops) {
    if (!stops || stops.length === 0) return 'linear-gradient(to right, #888, #888)';
    const sorted = [...stops].sort((a, b) => a.position - b.position);
    const parts = sorted.map(s => `${rgbToHex(s.color)} ${(s.position / 255 * 100).toFixed(1)}%`);
    return `linear-gradient(to right, ${parts.join(', ')})`;
}

const PaletteSelector = ({ instance, refreshKey }) => {
    const [palettes, setPalettes] = useState({});
    const [selected, setSelected] = useState('');

    useEffect(() => {
        setPalettes(getSavedPalettes());
    }, [refreshKey]);

    const onSelect = (e) => {
        const name = e.target.value;
        setSelected(name);
        if (name && palettes[name]) {
            applyPaletteToInstance(instance, palettes[name]);
        }
    };

    const names = Object.keys(palettes);

    return html`
        <div class="palette-selector">
            <label>Palette</label>
            <select value=${selected} onChange=${onSelect}>
                <option value="">-- default --</option>
                ${names.map(name => html`
                    <option key=${name} value=${name}>${name}</option>
                `)}
            </select>
            ${selected && palettes[selected] && html`
                <div class="palette-preview">
                    <div class="palette-gradient" style=${{ background: gradientCSS(palettes[selected].gradient) }}></div>
                    <div class="palette-swatches">
                        <div class="palette-swatch" style=${{ background: rgbToHex(palettes[selected].primary) }} title="primary"></div>
                        <div class="palette-swatch" style=${{ background: rgbToHex(palettes[selected].secondary) }} title="secondary"></div>
                        <div class="palette-swatch" style=${{ background: rgbToHex(palettes[selected].highlight) }} title="highlight"></div>
                    </div>
                </div>
            `}
        </div>
    `;
};

const PIXELMAP_STORAGE_KEY = 'hypervibe-custom-pixelmap';

const PixelMapLoader = ({ onScenesChange }) => {
    const fileRef = useRef(null);
    const hasCustom = !!localStorage.getItem(PIXELMAP_STORAGE_KEY);

    const loadFile = () => fileRef.current && fileRef.current.click();

    const onFileChange = (e) => {
        const file = e.target.files[0];
        if (!file) return;
        const reader = new FileReader();
        reader.onload = (ev) => {
            try {
                const data = JSON.parse(ev.target.result);
                if (!data.positions || !Array.isArray(data.positions)) {
                    alert('Invalid pixelmap: missing positions array');
                    return;
                }
                const pm = {
                    path: data.path || '/ws/monitor1',
                    instance: data.instance || '',
                    type: data.type || '2d',
                    size: data.size || 0.01,
                    positions: data.positions,
                };
                localStorage.setItem(PIXELMAP_STORAGE_KEY, JSON.stringify(pm));
                if (onScenesChange) onScenesChange([pm]);
            } catch (err) {
                alert('Failed to parse pixelmap file: ' + err.message);
            }
        };
        reader.readAsText(file);
        e.target.value = '';
    };

    const reset = () => {
        localStorage.removeItem(PIXELMAP_STORAGE_KEY);
        if (onScenesChange) onScenesChange([defaultPixelMap]);
    };

    return html`
        <div class="pixelmap-loader">
            <label>Pixel Map</label>
            <div class="pixelmap-buttons">
                <button onClick=${loadFile}>Load file</button>
                <button onClick=${reset} disabled=${!hasCustom}>Reset</button>
            </div>
            <input type="file" ref=${fileRef} accept=".json" style="display:none" onChange=${onFileChange} />
        </div>
    `;
};

export const Monitor = ({ scenes, wasmBinary, onScenesChange }) => {
    const canvasRef = useRef(null);
    const [wasmInstance, setWasmInstance] = useState(null);
    const [showPaletteEditor, setShowPaletteEditor] = useState(false);
    const [paletteRefreshKey, setPaletteRefreshKey] = useState(0);

    useEffect(async () => {
        if (!canvasRef.current) return;
        if (!wasmBinary) return;

        const config = { wasmBinary};

        wasmFactory(config).then(instance => {

            setWasmInstance(instance);

            // Keep track of allocated buffers to free them on unmount
            const wasmBuffers = [];
            let beatInterval;
            let exitFrameRequest = false;

            const createPixelSource = (scenePart, setBuffer) => {
                
                //set pixel buffer
                const sizeof_rgb = 3;
                const pixelBuffer = new Uint8Array(scenePart.positions.length * sizeof_rgb);
                const wasmBuffer = instance._malloc(pixelBuffer.length * pixelBuffer.BYTES_PER_ELEMENT);
                wasmBuffers.push(wasmBuffer);
                instance.HEAPU8.set(pixelBuffer, wasmBuffer);

                //set pixel map buffer
                const pixelMapData = new Float32Array(scenePart.positions.length * 2);
                for (let i = 0; i < scenePart.positions.length; i++) {
                    pixelMapData[i * 2] = scenePart.positions[i].x;
                    pixelMapData[i * 2 + 1] = scenePart.positions[i].y;
                }
                const pixelMapBuffer = instance._malloc(pixelMapData.length * pixelMapData.BYTES_PER_ELEMENT);
                wasmBuffers.push(pixelMapBuffer);
                instance.HEAPF32.set(pixelMapData, pixelMapBuffer / Float32Array.BYTES_PER_ELEMENT);

                //init
                instance._init(scenePart.positions.length, wasmBuffer, pixelMapBuffer);

                //run
                const updateFrame = () => {
                    instance._process();

                    const pixelBufferOut = instance.HEAPU8.subarray(wasmBuffer, wasmBuffer + pixelBuffer.length * pixelBuffer.BYTES_PER_ELEMENT);
                    setBuffer(pixelBufferOut);

                    if (!exitFrameRequest) {
                        requestAnimationFrame(updateFrame);
                    }
                };
                updateFrame();

                beatInterval = setInterval(() => instance._beat(), 500);
            };

            initWebGLMonitor(scenes, canvasRef.current, createPixelSource);
        });

        return () => {
            wasmBuffers.forEach(buffer => instance._free(buffer));
            clearInterval(beatInterval);
            exitFrameRequest = true;
            instance = null;
        }
    }, [scenes, wasmBinary]);

    const closePaletteEditor = () => {
        setShowPaletteEditor(false);
        setPaletteRefreshKey(k => k + 1);
    };

    return html`
        <div class="monitor-layout">
            <div class="monitor-controls">
                <${ParamSliders} instance=${wasmInstance} />
                <${PaletteSelector} instance=${wasmInstance} refreshKey=${paletteRefreshKey} />
                <div class="palette-editor-btn">
                    <button onClick=${() => setShowPaletteEditor(true)}>Edit Palettes</button>
                </div>
                <${PixelMapLoader} onScenesChange=${onScenesChange} />
            </div>
            <canvas ref=${canvasRef} width="640" height="480"></canvas>
        </div>
        ${showPaletteEditor && html`
            <div class="modal-overlay" onClick=${closePaletteEditor}>
                <div class="modal-content" onClick=${e => e.stopPropagation()}>
                    <button class="modal-close" onClick=${closePaletteEditor}>×</button>
                    <iframe src="palette/index.html"></iframe>
                </div>
            </div>
        `}
    `;
}