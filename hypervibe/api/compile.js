import fs from 'node:fs';
import { spawnSync } from 'node:child_process';

export const compile = (pattern) => {
    fs.writeFileSync('../compile/pattern.hpp', pattern);

    const result = spawnSync("./compile-server.sh", [], { cwd: '../compile', shell: true });

    // console.log('Compile stdout:', result.stdout.toString());
    // console.error('Compile stderr:', result.stderr.toString());

    if (result.status !== 0) {
        return result.stderr.toString();
    } else {
        return null;
    }

}