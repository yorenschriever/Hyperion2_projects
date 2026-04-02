import { html, useState, useEffect, useRef} from './common/preact-standalone.js'
import { main as initWebGLMonitor } from './monitor/webgl-monitor.js';
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

export const Monitor = ({ scenes, wasmBinary }) => {
    const canvasRef = useRef(null);
    const [wasmInstance, setWasmInstance] = useState(null);

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

    return html`
        <canvas ref=${canvasRef} width="640" height="480"></canvas>
        <${ParamSliders} instance=${wasmInstance} />
    `;
}