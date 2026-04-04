import {html, useState, useEffect, useRef} from './common/preact-standalone.js'
import {Monitor} from './monitor.js';
import {pixelMap as defaultPixelMap} from './pixelMap.js';

const PIXELMAP_STORAGE_KEY = 'hypervibe-custom-pixelmap';

function loadSavedPixelMap() {
    try {
        const saved = localStorage.getItem(PIXELMAP_STORAGE_KEY);
        if (saved) return JSON.parse(saved);
    } catch {}
    return null;
}

export const HypervibeApp = () => {
    const [scenes, setScenes] = useState(() => {
        const saved = loadSavedPixelMap();
        return [saved || defaultPixelMap];
    });
    const [promptState,setPromptState] = useState('');
    const [codeState,setCodeState] = useState('');
    const [wasmState,setWasmState] = useState(null);

    const [codeLoading, setCodeLoading] = useState(false);
    const [wasmLoading, setWasmLoading] = useState(false);
    // const [codeError, setCodeError] = useState(null);
    const [wasmError, setWasmError] = useState(null);

    const createCodeFromPrompt = async () => {
        setCodeLoading(true);
        setWasmLoading(true);
        try {
            const response = await fetch('/api/generate', {
                cache: "no-cache",
                method: "POST",
                body:promptState
            });
            const code = await response.text();
            console.log('Generated code:', code);
            setCodeState(code);
            setCodeLoading(false);
            await loadMonitorFromCode(code);
        } finally {
            setCodeLoading(false);
            setWasmLoading(false);
        }
    };

    const loadMonitorFromCode = async (codeStateArg) => {
        if (!codeStateArg) 
            return;
        setWasmLoading(true);
        setWasmError(null);
        try {
            const response = await fetch('/api/compile',{
                cache: "no-cache",
                method: "POST",
                body: codeStateArg
            });
            const wasmBinary = await response.arrayBuffer();
            const view = new Uint8Array(wasmBinary);
            if (wasmBinary.byteLength === 0) {
                setWasmError("Empty WASM binary");
                setWasmState(null);
            } else if ((view[0] != 0x00 || view[1] != 0x61 || view[2] != 0x73 || view[3] != 0x6D)) {
                const errorContent = await new TextDecoder().decode(wasmBinary);
                console.error('Invalid WASM binary:', errorContent);
                // console.log('Received WASM binary (hex):', Array.from(new Uint8Array(wasmBinary)).map(b => b.toString(16).padStart(2, '0')).join(' '));
                setWasmError( errorContent );
                setWasmState(null);
            } else {
                setWasmState(wasmBinary);
            }
        } catch (error) {
            console.error('Error loading monitor from code:', error);
            setWasmError(error);
        } finally {
            setWasmLoading(false);
        }
    }

    const download = (code) => {
        //filter the name from the code, assuming it's in the format: class CelestialDrift : public Pattern<RGBA>
        const nameMatch = code.match(/class\s+(\w+)\s*:\s*public Pattern<RGBA>/);
        const name = nameMatch ? nameMatch[1] : 'pattern';

        const blob = new Blob([code], { type: 'text/plain' });
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = `${name}.cpp`;
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);
        URL.revokeObjectURL(url);
    };

    const promptKeyDownHandler = (e) => {
        if (codeLoading) return; 
        if ((e.key === 'Enter' || e.key ==='s') && (e.ctrlKey || e.metaKey)) 
            createCodeFromPrompt() 
    }

    const codeKeyDownHandler = (e) => {
        if (wasmLoading) return; 
        if ((e.key === 'Enter' || e.key ==='s') && (e.ctrlKey || e.metaKey)) 
            loadMonitorFromCode(codeState);
    }

    const onScenesChange = (newScenes) => {
        setScenes(newScenes);
    };

    const [glossaryOpen, setGlossaryOpen] = useState(false);
    const toggleGlossary = () => setGlossaryOpen(v => !v)

    return html`
        <div class="vibe-patterns-app">
            ${glossaryOpen && html`<${GlossaryModal} onClose=${toggleGlossary}/>`}

            <div class="monitor">
                <${Monitor} scenes=${scenes} wasmBinary=${wasmState} key=${wasmState} onScenesChange=${onScenesChange}/>
                ${wasmLoading && html`<${LoadingSpinner}/>`}
                ${wasmError && html`<${ErrorIcon} text=${wasmError}/>`}
            </div>
        
            <div class="prompt-editor">
                <textarea value=${promptState} onInput=${ (e) => setPromptState(e.target.value)} onKeyDown=${promptKeyDownHandler}></textarea>
                <div class="buttons">
                    <button onClick=${ () => toggleGlossary()} disabled=${codeLoading}>Glossary</button>
                    <button onClick=${ () => createCodeFromPrompt()} disabled=${codeLoading}>Generate code</button>
                </div>
            </div>
    
            <div class="code-editor">
                <textarea value=${codeState} onInput=${ (e) => setCodeState(e.target.value)} onKeyDown=${codeKeyDownHandler}></textarea>
                <div class="buttons">
                    <button onClick=${ () => loadMonitorFromCode(codeState)} disabled=${wasmLoading}>Compile</button>
                    <button onClick=${ () => download(codeState)}>Download</button>
                </div>
                ${codeLoading && html`<${LoadingSpinner}/>`}
            </div>
        </div>
    `
}

const LoadingSpinner = () => html`
    <div class="loading-spinner">
        <div class="spinner"></div>
    </div>
`;

const ErrorIcon = ({text}) => html`
    <div class="error-icon">
        <svg viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
            <circle cx="12" cy="12" r="10" stroke="currentColor" stroke-width="2"/>
            <line x1="8" y1="8" x2="16" y2="16" stroke="currentColor" stroke-width="2"/>
            <line x1="16" y1="8" x2="8" y2="16" stroke="currentColor" stroke-width="2"/>
        </svg>
        <div class="error-text">${text || "Error"}</div>
    </div>
`;

const glossaryItems = [
    { term: 'Palette', description: 'Has a primary color, secondary color, highlight color, and gradient. You cannot use colors outside of this palette.' },
    { term: 'Chaser', description: 'A wave moving through the leds, based on pixel indices.' },
    { term: 'Sweep', description: 'A wave moving through the leds, based on their position in the map.' },
    { term: 'Angular', description: 'Rotating, based on the angle of the polar coordinates.' },
    { term: 'Radial', description: 'Growing from the center, based on the radius of the polar coordinates.' },
    { term: 'Strobe', description: 'All leds on and off at the same time, fast.' },
    { term: 'Blinder', description: 'Very bright when the pattern is on, fade out slowly when the pattern is off.' },
    { term: 'Flash', description: 'Very bright on a beat, fade out quickly after that.' },
    { term: 'Glow', description: 'Lets pixels will breathe to full brightness and back, with a break in between.' },
    { term: 'LFO', description: 'Oscillator. Shapes: SawUp, SawDown, SinFast, Glow, PWM, Tri, SoftPwm, SoftSawUp, SoftSawDown. The soft variants are smoother versions of the standard shapes. LFOs can be synced to the music tempo or have independent timing.' },
    { term: 'On Beat', description: 'Trigger things on the beat of the music.' },
    { term: 'Transition', description: 'The Fade in/out animation when a pattern is turned on or off.' },
    { term: 'Fade', description: 'Value that goes from 1 to 0 over time. Variants: FadeUp, FadeDown. Easing: Linear, Quadratic, Cubic.' },
    { term: 'Params', description: 'Values tweakable by the user: velocity, amount, size, variant, offset, intensity.' },
    { term: 'Particle system', description: 'Draw many things on screen that move around and interact. Good for fire, smoke, sparks, etc.' },
];

const GlossaryModal = ({onClose}) => html`
    <div class="modal-overlay" onClick=${onClose}>
        <div class="modal-content glossary-modal" onClick=${(e) => e.stopPropagation()}>
            <button class="modal-close" onClick=${onClose}>×</button>
            <div class="glossary-body">
                <h2>Glossary</h2>
                <table class="glossary-table">
                    <thead><tr><th>Term</th><th>Description</th></tr></thead>
                    <tbody>
                        ${glossaryItems.map(item => html`
                            <tr><td>${item.term}</td><td>${item.description}</td></tr>
                        `)}
                    </tbody>
                </table>
                ${glossaryItems.length === 0 && html`<p class="glossary-empty">No glossary items yet.</p>`}
            </div>
        </div>
    </div>
`; 
