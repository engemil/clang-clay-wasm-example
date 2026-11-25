import { readFile } from 'node:fs/promises';
import { createServer } from 'node:http';
import { resolve } from 'node:path';
import { chromium } from 'playwright-core';
import { testCounter } from './counter_test.mjs';

export async function runBrowserCounter(buildDir, { html, afterTest } = {}) {
    const pageHtml = html ?? await readFile(resolve(buildDir, 'index.html'));
    const wasm = await readFile(resolve(buildDir, 'index.wasm'));
    const server = createServer((req, res) => {
        if (req.url === '/') {
            res.writeHead(200, { 'Content-Type': 'text/html; charset=utf-8' }).end(pageHtml);
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
        await afterTest?.(page);
        console.log('Browser counter test passed.');
    } finally {
        try {
            await browser?.close();
        } finally {
            server.close();
        }
    }
}
