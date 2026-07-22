/**
 * Given a mapping function f(...x) => ...y, returns a method for transforming a function's signature
 * from f(...y) => returnType to f(...x) => returnType.
 *
 * @example
 * const concat = (x: string, y: string): string => x + y;
 * const concatNumbers = mapArguments((i: number, j: number): [string, string] => [i.toString(), j.toString()])(concat);
 * concatNumbers(123, 456); // returns "123456"
 *
 * @param mapper maps the desired input arguments to the required arguments
 * @returns a function that updates the arguments of the input function to the desired argument types
 */
// ReturnType is a type parameter of the returned function rather than of `mapArguments` itself,
// because the mapper never constrains it. Deferring it until `fn` is applied lets one mapped wrapper
// be reused with functions of different return types, and lets the compiler infer ArgsOut as a tuple
// in pipe/compose chains instead of widening a mapper's array literal (e.g. `x => [a]`) to Array<a>.
export function mapArguments<ArgsIn extends unknown[], ArgsOut extends unknown[]>(
  mapper: (...args: ArgsIn) => ArgsOut,
): <ReturnType>(fn: (...args: ArgsOut) => ReturnType) => (...args: ArgsIn) => ReturnType {
  return fn =>
    (...args) =>
      fn(...mapper(...args));
}
