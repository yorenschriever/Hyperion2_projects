import {html, useState, useEffect, useRef} from './common/preact-standalone.js'
import {Monitor} from './monitor.js';
import {pixelMap} from './pixelMap.js';

const scenes = [pixelMap];

export const HypervibeApp = () => {
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
            const response = await fetch('/generate', {
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
            const response = await fetch('/compile',{
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

    return html`
        <div class="vibe-patterns-app">

            <div class="monitor">
                <${Monitor} scenes=${scenes} wasmBinary=${wasmState} key=${wasmState}/>
                ${wasmLoading && html`<${LoadingSpinner}/>`}
                ${wasmError && html`<${ErrorIcon} text=${wasmError}/>`}
            </div>
        
            <div class="prompt-editor">
                <textarea value=${promptState} onInput=${ (e) => setPromptState(e.target.value)} onKeyDown=${promptKeyDownHandler}></textarea>
                <div class="buttons">
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
