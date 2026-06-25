/**
 * ts-loader integration for JSXProcessor via getCustomTransformers API.
 *
 * Usage in webpack.config.js:
 *
 *   const { createValdiJSXTransformers } = require('valdi-compiler-js/src/valdiJSXWebpackTransformer');
 *
 *   {
 *     loader: 'ts-loader',
 *     options: {
 *       transpileOnly: false,
 *       getCustomTransformers: (program) => createValdiJSXTransformers(program),
 *     }
 *   }
 *
 * Requires typescript as a peer dependency — the consumer's typescript
 * must be the same instance used by ts-loader. This is satisfied by
 * valdi-compiler-js declaring typescript as a peerDependency.
 */

import * as ts from 'typescript';
import { JSXProcessor } from './JSXProcessor';

function fileContainsJSX(sourceFile: ts.SourceFile): boolean {
  let found = false;
  function visit(node: ts.Node) {
    if (found) return;
    if (ts.isJsxElement(node) || ts.isJsxSelfClosingElement(node) || ts.isJsxFragment(node)) {
      found = true;
      return;
    }
    ts.forEachChild(node, visit);
  }
  visit(sourceFile);
  return found;
}

export function createValdiJSXTransformers(program: ts.Program): ts.CustomTransformers {
  const typeChecker = program.getTypeChecker();

  const perFileTransformer: ts.TransformerFactory<ts.SourceFile> = (context) => {
    return (sourceFile) => {
      if (!fileContainsJSX(sourceFile)) {
        return sourceFile;
      }

      const processor = new JSXProcessor();
      const transforms = processor.makeTransformers(typeChecker);

      let result = sourceFile;
      for (const factory of transforms) {
        const transform = factory(context);
        result = transform(result);
      }
      return result;
    };
  };

  return {
    before: [perFileTransformer],
  };
}
