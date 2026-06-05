import * as ts from 'typescript';

/**
 * TypeScript transformer that annotates source-level variable-arg require()
 * calls with a /* @valdi-dynamic *​/ comment.
 *
 * In the TypeScript AST (before tsc's module transform):
 * - `import X from "Y"` → ImportDeclaration node (NOT a require call)
 * - `require("Y")` in source → CallExpression with Identifier("require")
 *
 * This transformer only annotates CallExpressions where the first argument
 * is NOT a string literal (variable/dynamic requires). String-literal
 * requires are left untouched — they work on all platforms as-is.
 *
 * The annotation is consumed by PrependWebJsProcessor (web-only) which
 * converts the annotated require to globalThis.moduleLoader.load().
 * On native, the minifier strips the comment — zero behavior change.
 */

export function createWebRequireTransformer(): ts.TransformerFactory<ts.SourceFile> {
  return (context: ts.TransformationContext): ts.Transformer<ts.SourceFile> => {
    const factory = context.factory;

    return (sourceFile: ts.SourceFile): ts.SourceFile => {
      const visitor: ts.Visitor = (node: ts.Node): ts.VisitResult<ts.Node> => {
        const visitedNode = ts.visitEachChild(node, visitor, context);

        if (
          ts.isCallExpression(visitedNode) &&
          ts.isIdentifier(visitedNode.expression) &&
          visitedNode.expression.text === 'require' &&
          visitedNode.arguments.length >= 1
        ) {
          const firstArg = visitedNode.arguments[0];

          // String literal (including no-substitution template literals) —
          // leave untouched for all platforms
          if (ts.isStringLiteralLike(firstArg)) {
            return visitedNode;
          }

          // Variable first arg — annotate for web-only transform.
          // PrependWebJsProcessor converts to moduleLoader.load().
          // Native minifier strips the comment.
          // Spread args to force a new node — updateCallExpression returns the
          // original if children are identity-equal, and addSyntheticLeadingComment
          // mutates in place (would accumulate in watch mode).
          const freshNode = factory.updateCallExpression(
            visitedNode,
            visitedNode.expression,
            visitedNode.typeArguments,
            [...visitedNode.arguments],
          );
          ts.addSyntheticLeadingComment(
            freshNode,
            ts.SyntaxKind.MultiLineCommentTrivia,
            ' @valdi-dynamic ',
            /* hasTrailingNewLine */ false,
          );
          return freshNode;
        }

        return visitedNode;
      };

      return ts.visitNode(sourceFile, visitor) as ts.SourceFile;
    };
  };
}
