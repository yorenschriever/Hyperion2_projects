import { html, useState, useEffect, useRef} from './common/preact-standalone.js'
import { main as initWebGLMonitor } from './monitor/webgl-monitor.js';
import wasmFactory from './pattern.mjs';

export const Monitor = ({ scenes, code }) => {
    const canvasRef = useRef(null);

    useEffect(async () => {
        if (!canvasRef.current) return;

        console.log('Compiling code for monitor:', code);
        const config = {
            wasmBinary: await fetch('/compile',{cache: "no-cache", method: "POST", body: code}).then(response => response.arrayBuffer())
        };

        wasmFactory(config).then(instance => {

            // Keep track of allocated buffers to free them on unmount
            const wasmBuffers = [];
            let beatInterval;
            let exitFrameRequest = false;

            const createPixelSource = (scenePart, setBuffer) => {
                
                const sizeof_rgb = 3;
                const pixelBuffer = new Uint8Array(scenePart.positions.length * sizeof_rgb);
                const wasmBuffer = instance._malloc(pixelBuffer.length * pixelBuffer.BYTES_PER_ELEMENT);
                wasmBuffers.push(wasmBuffer);
                instance.HEAPU8.set(pixelBuffer, wasmBuffer);

                instance._init(scenePart.positions.length, wasmBuffer);

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
    }, [scenes, code]);

    return html`<canvas ref=${canvasRef} width="640" height="480"></canvas>`;
}