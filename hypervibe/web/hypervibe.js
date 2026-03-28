import {html, useState, useEffect, useRef} from './common/preact-standalone.js'
import {Monitor} from './monitor.js';
import {pixelMap} from './pixelMap.js';

const scenes = [pixelMap];

export const HypervibeApp = () => {
    const [promptState,setPromptState] = useState('');
    const [codeState,setCodeState] = useState('');
    const [monitorState,setMonitorState] = useState('');

    const createCodeFromPrompt = async () => {
        const response = await fetch('/generate', {
            cache: "no-cache",
            method: "POST",
            body:promptState
        });
        const code = await response.text();
        console.log('Generated code:', code);
        setCodeState(code);
        setMonitorState(code);
    }
    ;

    return html`
        <div class="vibe-patterns-app">
            <div class="monitor">
                <${Monitor} scenes=${scenes} code=${monitorState} key=${monitorState}/>
            </div>
        
            <span class="monitor-name">${pixelMap.instance}</span>

            <textarea value=${promptState} onInput=${ (e) => setPromptState(e.target.value)}></textarea>
            <button onClick=${ () => createCodeFromPrompt()}>Generate code</button>
            <textarea value=${codeState} onInput=${ (e) => setCodeState(e.target.value)}></textarea>
            <button onClick=${ () => setMonitorState(codeState)}>Update Monitor</button>
        </div>
    `
}
