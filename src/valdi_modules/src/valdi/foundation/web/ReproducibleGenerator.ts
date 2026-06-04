/**
 * Web stub for foundation/test/util/ReproducibleGenerator.
 *
 * Surface mirrors the real generator so non-test callers compiled into the
 * web npm package (e.g. composer_example/TextCatalog) type-check and don't
 * hit missing methods at runtime. All methods return deterministic constants
 * — no randomness, no faker dep on web.
 */
export class ReproducibleGenerator {
  constructor(_seed: number) {}

  nextUniqueId(): string {
    return '00000000-0000-0000-0000-000000000000';
  }
  nextInternetName(_locale?: string): string {
    return 'stub';
  }
  nextFreeformName(_locale?: string): string {
    return 'stub';
  }
  nextFreeformWords(_locale?: string): string {
    return 'stub';
  }
  nextPhoneNumber(_locale?: string): string {
    return '000-000-0000';
  }
  nextAlphaNumericString(size: number): string {
    return '0'.repeat(size);
  }
  nextBoolean(): boolean {
    return false;
  }
  nextFloat(): number {
    return 0;
  }
  nextInt(): number {
    return 0;
  }
  nextRange(start: number, _end: number): number {
    return start;
  }
  nextPastTimestampMs(): number {
    return 0;
  }
  nextFriendmoji(): string {
    return '';
  }
  nextLocale(): string {
    return 'en';
  }
  availableLocales(): string[] {
    return [];
  }
}
