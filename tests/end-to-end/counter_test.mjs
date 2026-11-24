import assert from 'node:assert/strict';

export async function testCounter(page, url) {
    page.setDefaultTimeout(5000);
    const pageErrors = [];
    page.on('pageerror', error => pageErrors.push(error));
    // Advance the application's frame loop independently of the headless compositor.
    const start = Date.UTC(2025, 0, 1);
    await page.clock.install({ time: start });
    await page.clock.pauseAt(start + 60_000);
    await page.goto(url);

    const counter = page.locator('.text').filter({ hasText: /^\d+$/ });
    await counter.waitFor();
    assert.equal(await counter.textContent(), '0');

    const clickAndExpectCount = async (x, y, expected) => {
        await page.mouse.click(x, y);
        // One frame consumes the press; the next observes the release.
        await page.clock.runFor(32);
        assert.equal(await counter.textContent(), String(expected));
    };
    await clickAndExpectCount(1, 1, 0);
    const button = await page.getByText('Click me', { exact: true }).boundingBox();
    assert.ok(button && button.width > 0 && button.height > 0, 'Counter button is not visible');
    const x = button.x + button.width / 2;
    const y = button.y + button.height / 2;
    await clickAndExpectCount(x, y, 1);
    await clickAndExpectCount(x, y, 2);
    assert.deepEqual(pageErrors, []);
}
