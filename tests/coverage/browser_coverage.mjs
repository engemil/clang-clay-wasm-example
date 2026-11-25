import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import { resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import coverage from 'istanbul-lib-coverage';
import instrument from 'istanbul-lib-instrument';
import report from 'istanbul-lib-report';
import reports from 'istanbul-reports';
import { runBrowserCounter } from '../end-to-end/browser_runner.mjs';

const root = fileURLToPath(new URL('../../', import.meta.url));
const [buildDir, reportDir] = process.argv.slice(2);
const sourcePath = resolve(root, 'src/web/index.html');
const html = await readFile(resolve(buildDir, 'index.html'), 'utf8');
const scripts = [...html.matchAll(/<script type="module">([\s\S]*?)<\/script>/g)];
assert.equal(scripts.length, 1, 'Expected one inline renderer module');
const [{ 0: tag, 1: source, index }] = scripts;
// Preserve the renderer's original HTML line numbers in the coverage map.
const padding = '\n'.repeat(html.slice(0, index).split('\n').length - 1);
const code = instrument.createInstrumenter({ esModules: true }).instrumentSync(padding + source, sourcePath);
const instrumented = html.slice(0, index) + `<script type="module">${code}</script>` + html.slice(index + tag.length);

await runBrowserCounter(buildDir, { html: instrumented, afterTest: async page => {
    const data = await page.evaluate(() => globalThis.__coverage__);
    assert.ok(data?.[sourcePath], 'Browser did not record renderer coverage');
    const coverageMap = coverage.createCoverageMap(data);
    const context = report.createContext({ dir: reportDir, coverageMap });
    reports.create('lcovonly', { file: 'browser.lcov', projectRoot: root }).execute(context);
} });
