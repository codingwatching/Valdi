import 'ts-jest';
import * as ts from 'typescript';
import { createWebRequireTransformer } from './WebRequireTransformer';
import { trimAllLines } from './utils/StringUtils';

function sanitize(text: string): string {
  return trimAllLines(text);
}

function transform(input: string): string {
  const result = ts.transpileModule(input, {
    compilerOptions: {
      target: ts.ScriptTarget.ES2019,
      module: ts.ModuleKind.CommonJS,
    },
    transformers: {
      before: [createWebRequireTransformer()],
    },
  });
  return sanitize(result.outputText);
}

const MARKER = '/* @valdi-dynamic */';

describe('WebRequireTransformer', () => {
  describe('string literal requires are untouched', () => {
    it('keeps single-arg string require', () => {
      const result = transform(`const x = require("DeviceBridge");`);
      expect(result).toContain('require("DeviceBridge")');
      expect(result).not.toContain(MARKER);
    });

    it('keeps multi-arg string require', () => {
      const result = transform(`const x = require("DeviceBridge", true);`);
      expect(result).toContain('require("DeviceBridge", true)');
      expect(result).not.toContain(MARKER);
    });

    it('keeps relative path require', () => {
      const result = transform(`const x = require("./utils/helper");`);
      expect(result).toContain('require("./utils/helper")');
      expect(result).not.toContain(MARKER);
    });

    it('keeps template literal require without substitutions', () => {
      const result = transform('const x = require(`DeviceBridge`);');
      expect(result).not.toContain(MARKER);
    });
  });

  describe('variable requires are annotated', () => {
    it('annotates single-arg variable require', () => {
      const result = transform(`const x = require(modulePath);`);
      expect(result).toContain(`${MARKER} require(modulePath)`);
    });

    it('annotates multi-arg variable require preserving all args', () => {
      const result = transform(`const x = require(componentPath, true, true);`);
      expect(result).toContain(`${MARKER} require(componentPath, true, true)`);
    });
  });

  describe('import-generated requires are untouched', () => {
    it('import statement becomes plain require via tsc', () => {
      const result = transform(`import { Component } from "valdi_core/src/Component";\nconsole.log(Component);`);
      expect(result).toContain('require("valdi_core/src/Component")');
      expect(result).not.toContain(MARKER);
    });
  });

  describe('non-require calls are untouched', () => {
    it('ignores other function calls', () => {
      const result = transform(`const x = someFunction("test");`);
      expect(result).not.toContain(MARKER);
    });

    it('ignores require.resolve', () => {
      const result = transform(`const x = require.resolve("path");`);
      expect(result).not.toContain(MARKER);
    });
  });

  describe('mixed requires in one file', () => {
    it('only annotates variable requires', () => {
      const input = `
        const a = require("DeviceBridge");
        const b = require(dynamicPath, true);
        const c = require("Cof", true);
      `;
      const result = transform(input);
      expect(result).toContain('require("DeviceBridge")');
      expect(result).toContain(`${MARKER} require(dynamicPath, true)`);
      expect(result).toContain('require("Cof", true)');
      // Only one annotation
      expect((result.match(/@valdi-dynamic/g) || []).length).toBe(1);
    });
  });
});
