import type { Argv } from 'yargs';
import * as fs from 'fs';
import * as path from 'path';
import * as os from 'os';
import { makeCommandHandler } from '../../utils/errorUtils';
import type { ArgumentsResolver } from '../../utils/ArgumentsResolver';
import { connectToDaemon, resolveClientId, resolveContextId, DEFAULT_PORT } from '../../utils/daemonClient';
import { ANSI_COLORS } from '../../core/constants';
import { wrapInColor } from '../../utils/logUtils';

interface CommandParameters {
  elementId: string;
  contextId: string | undefined;
  port: number;
  client: string | undefined;
  output: string | undefined;
}

async function inspectSnapshot(argv: ArgumentsResolver<CommandParameters>) {
  const elementId = argv.getArgument('elementId') as string;
  const contextIdArg = argv.getArgument('contextId') as string | undefined;
  const port = argv.getArgument('port') as number;
  const clientOverride = argv.getArgument('client') as string | undefined;
  const outputOverride = argv.getArgument('output') as string | undefined;

  const conn = await connectToDaemon(port);
  try {
    await conn.configure();
    const clientId = await resolveClientId(conn, clientOverride);
    const contextId = await resolveContextId(conn, clientId, contextIdArg);
    const base64Data = await conn.takeSnapshot(clientId, elementId, contextId);

    const outPath = outputOverride
      ? path.resolve(outputOverride)
      : path.join(os.tmpdir(), `valdi-snapshot-${elementId}.png`);

    fs.writeFileSync(outPath, Buffer.from(base64Data, 'base64'));
    console.log(JSON.stringify({ path: outPath }));
    console.error(wrapInColor(`Screenshot saved: ${outPath}`, ANSI_COLORS.GREEN_COLOR));
  } finally {
    conn.close();
  }
}

export const command = 'snapshot <elementId> [contextId]';
export const describe = 'Capture a screenshot of an element and write it to a PNG file';
export const builder = (yargs: Argv<CommandParameters>) => {
  yargs
    .positional('elementId', {
      describe: 'Element ID to screenshot',
      type: 'string',
      demandOption: true,
    })
    .positional('contextId', {
      describe: 'Context ID containing the element (omit to auto-select or be prompted)',
      type: 'string',
    })
    .option('port', {
      describe: 'Daemon TCP port',
      type: 'number',
      default: DEFAULT_PORT,
    })
    .option('client', {
      describe: 'Client ID to target (from "valdi inspect devices")',
      type: 'string',
    })
    .option('output', {
      describe: 'Output PNG file path (defaults to /tmp/valdi-snapshot-<elementId>.png)',
      type: 'string',
      alias: 'o',
    });
};
export const handler = makeCommandHandler(inspectSnapshot);
