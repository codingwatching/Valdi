import { mapArguments } from 'foundation/src/functional/mapArguments';
import 'jasmine/src/jasmine';

describe('mapArguments', () => {
  it('maps function arguments', () => {
    const concat = (x: string, y: string): string => x + y;
    const concatNumbers = mapArguments((i: number, j: number): [string, string] => [i.toString(), j.toString()])(
      concat,
    );
    expect(concatNumbers(123, 456)).toEqual('123456');
  });

  it('infers a tuple from a bare array-literal mapper (no `as const` or return annotation needed)', () => {
    const formatId = (value: { id: number }): string => `#${value.id}`;
    // The annotation is the assertion: a bare `[{ id }]` mapper must yield `(id: number) => string`,
    // not widen to a rest-parameter signature.
    const mapped: (id: number) => string = mapArguments((id: number) => [{ id }])(formatId);
    expect(mapped(7)).toEqual('#7');
  });

  it('preserves each wrapped function return type (inferred at application, not creation)', () => {
    const remap = mapArguments((id: number) => [{ id }]);
    // The same mapped wrapper is reused with functions of different return types; both annotations
    // must hold, which only works when ReturnType is deferred to the wrapped function.
    const toLabel: (id: number) => string = remap((value: { id: number }) => `#${value.id}`);
    const toId: (id: number) => number = remap((value: { id: number }) => value.id);
    expect(toLabel(7)).toEqual('#7');
    expect(toId(7)).toEqual(7);
  });
});
