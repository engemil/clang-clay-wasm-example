import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import { createServer } from 'node:http';
import { resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { chromium } from 'playwright-core';
import coverage from 'istanbul-lib-coverage';
import instrument from 'istanbul-lib-instrument';
import report from 'istanbul-lib-report';
import reports from 'istanbul-reports';
import { testCounter } from '../end-to-end/counter_test.mjs';

const root = fileURLToPath(new URL('../../', import.meta.url));
const [buildDir, reportDir] = process.argv.slice(2);
const sourcePath = resolve(root, 'src/web/index.html');
const html = await readFile(sourcePath, 'utf8');
const wasm = await readFile(resolve(buildDir, 'index.wasm'));
const scripts = [...html.matchAll(/<script type="module">([\s\S]*?)<\/script>/g)];
assert.equal(scripts.length, 1, 'Expected one inline renderer module');
const [{ 0: tag, 1: source, index }] = scripts;
// Preserve the renderer's original HTML line numbers in the coverage map.
const padding = '\n'.repeat(html.slice(0, index).split('\n').length - 1);
const code = instrument.createInstrumenter({ esModules: true }).instrumentSync(padding + source, sourcePath);
const instrumented = html.slice(0, index) + `<script type="module">${code}</script>` + html.slice(index + tag.length);

const server = createServer((req, res) => {
    if (req.url === '/') {
        res.writeHead(200, { 'Content-Type': 'text/html; charset=utf-8' }).end(instrumented);
    } else if (req.url === '/index.wasm') {
        res.writeHead(200, { 'Content-Type': 'application/wasm' }).end(wasm);
    } else {
        res.writeHead(404).end();
    }
});
await new Promise((resolve, reject) => server.once('error', reject).listen(0, '127.0.0.1', resolve));
let browser;
try {
    browser = await chromium.launch({ executablePath: process.env.CHROMIUM });
    const page = await browser.newPage();
    await testCounter(page, `http://127.0.0.1:${server.address().port}/`);

    const data = await page.evaluate(() => globalThis.__coverage__);
    assert.ok(data?.[sourcePath], 'Browser did not record renderer coverage');
    const coverageMap = coverage.createCoverageMap(data);
    const context = report.createContext({ dir: reportDir, coverageMap });
    reports.create('lcovonly', { file: 'browser.lcov', projectRoot: root }).execute(context);
    console.log('Browser counter test passed.');
} finally {
    try {
        await browser?.close();
    } finally {
        server.close();
    }
}
