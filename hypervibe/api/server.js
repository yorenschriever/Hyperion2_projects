import express from 'express'
import {compile} from './compile.js'
import {generate} from './generate.js'
import {generatePalette} from './generatePalette.js'

const app = express();

app.use(function(req, res, next) {
  var contentType = req.headers['content-type'] || ''
    , mime = contentType.split(';')[0];

  if (mime != 'text/plain') {
    return next();
  }

  var data = '';
  req.setEncoding('utf8');
  req.on('data', function(chunk) {
    data += chunk;
  });
  req.on('end', function() {
    req.rawBody = data;
    next();
  });
});

// Trust proxy - important for getting real client IP
app.set('trust proxy', true);

app.get('/', (req, res) => {
    res.json({
        message: 'Hello World',
        ip: req.ip,
        protocol: req.protocol
    });
});

app.get('/api/health', (req, res) => {
    res.json({
        status: 'ok2',
        ip: req.ip,
        protocol: req.protocol
    });
});

app.post('/api/compile', async (req, res) => {
    const error  = await compile(req.rawBody)

    if (error) {
        res.status(200).send(error);
        return;
    }

    res.status(200).sendFile("compile/pattern.wasm", { root: '..' });
});

app.post('/api/generate', async (req, res) => {
    res.status(200).send(await generate(req.rawBody));
});

app.post('/api/generate-palette', async (req, res) => {
    res.status(200).send(await generatePalette(req.rawBody));
});

const PORT = process.env.PORT || 3000;
app.listen(PORT, '127.0.0.1', () => {
    console.log(`Server running on port ${PORT}`);
});