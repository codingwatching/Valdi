import 'jasmine';
import * as fs from 'fs';
import * as path from 'path';
import * as os from 'os';
import { getAdapterByName } from './skillsAdapters';

describe('ClaudeCodeAdapter', () => {
  let tmpHome: string;
  let origHome: string;

  beforeEach(() => {
    tmpHome = fs.mkdtempSync(path.join(os.tmpdir(), 'valdi-test-'));
    origHome = process.env['HOME']!;
    process.env['HOME'] = tmpHome;
    fs.mkdirSync(path.join(tmpHome, '.claude'), { recursive: true });
  });

  afterEach(() => {
    process.env['HOME'] = origHome;
    fs.rmSync(tmpHome, { recursive: true, force: true });
  });

  function getAdapter() {
    const adapter = getAdapterByName('claude');
    expect(adapter).toBeDefined();
    return adapter!;
  }

  it('writes SKILL.md with frontmatter to ~/.claude/skills/', () => {
    const adapter = getAdapter();
    adapter.install('my-skill', '# My content', { name: 'my-skill', description: 'Desc', tags: [], path: '', category: [] });

    const skillPath = path.join(tmpHome, '.claude', 'skills', 'my-skill', 'SKILL.md');
    expect(fs.existsSync(skillPath)).toBe(true);

    const content = fs.readFileSync(skillPath, 'utf8');
    expect(content).toContain('name: my-skill');
    expect(content).toContain('description: Desc');
    expect(content).toContain('# My content');
  });

  it('does not modify settings.json', () => {
    const settingsFile = path.join(tmpHome, '.claude', 'settings.json');
    fs.writeFileSync(settingsFile, JSON.stringify({ model: 'claude-opus-4-6' }), 'utf8');

    const adapter = getAdapter();
    adapter.install('test-skill', '# Content', { name: 'test-skill', description: 'Desc', tags: [], path: '', category: [] });

    const settings = JSON.parse(fs.readFileSync(settingsFile, 'utf8'));
    expect(settings).toEqual({ model: 'claude-opus-4-6' });
  });

  it('lists installed skills', () => {
    const adapter = getAdapter();
    adapter.install('skill-a', '# A', { name: 'skill-a', description: 'A', tags: [], path: '', category: [] });
    adapter.install('skill-b', '# B', { name: 'skill-b', description: 'B', tags: [], path: '', category: [] });

    const installed = adapter.listInstalled();
    expect(installed).toContain('skill-a');
    expect(installed).toContain('skill-b');
  });

  it('removes a skill', () => {
    const adapter = getAdapter();
    adapter.install('to-remove', '# Remove me', { name: 'to-remove', description: 'R', tags: [], path: '', category: [] });
    expect(adapter.listInstalled()).toContain('to-remove');

    adapter.remove('to-remove');
    expect(adapter.listInstalled()).not.toContain('to-remove');
  });
});
