import fs from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";
import Ajv from "ajv";

const scriptFile = fileURLToPath(import.meta.url);
const scriptDir = path.dirname(scriptFile);
const airtraceRoot = path.resolve(scriptDir, "..");
const docsRoot = path.join(airtraceRoot, "docs");
const schemaPath = path.join(docsRoot, "sources_menu_registry.schema.json");
const registryPath = path.join(docsRoot, "sources_menu_registry.json");
const checkMode = process.argv.includes("--check");

function fail(message) {
  throw new Error(message);
}

function readJson(filePath) {
  try {
    return JSON.parse(fs.readFileSync(filePath, "utf-8"));
  } catch (error) {
    fail(`failed to parse JSON at ${path.relative(airtraceRoot, filePath)}: ${error.message}`);
  }
}

function escapeRegExp(value) {
  return value.replace(/[.*+?^${}()|[\]\\]/g, "\\$&");
}

function normalizePath(filePath) {
  return path.relative(airtraceRoot, filePath).replace(/\\/g, "/");
}

function detectNewline(text) {
  return text.includes("\r\n") ? "\r\n" : "\n";
}

function renderEntry(entry) {
  if (entry.url) {
    return `- ${entry.label}: ${entry.url}`;
  }
  const linkText = entry.links.map((link) => `${link.label}=${link.url}`).join("; ");
  return `- ${entry.label}: ${linkText}`;
}

function sortByOrderThenId(items, idKey) {
  return [...items].sort((left, right) => {
    if (left.order !== right.order) {
      return left.order - right.order;
    }
    return String(left[idKey]).localeCompare(String(right[idKey]));
  });
}

function validateRegistry(schema, registry) {
  const ajv = new Ajv({ allErrors: true, strict: false });
  const validate = ajv.compile(schema);
  const valid = validate(registry);
  if (valid) {
    return;
  }

  const errors = validate.errors
    .map((error) => {
      const instancePath = error.instancePath || "(root)";
      return `${instancePath} ${error.message}`;
    })
    .join("\n");
  fail(`registry validation failed:\n${errors}`);
}

function applyGeneratedSections(registry) {
  const fileState = new Map();
  const sections = sortByOrderThenId(registry.generated_sections, "section_id");

  for (const section of sections) {
    const sourcePath = path.join(airtraceRoot, section.source_file);
    if (!fs.existsSync(sourcePath)) {
      fail(`source file does not exist: ${section.source_file}`);
    }

    if (!fileState.has(sourcePath)) {
      const original = fs.readFileSync(sourcePath, "utf-8");
      fileState.set(sourcePath, {
        original,
        updated: original,
        newline: detectNewline(original),
      });
    }

    const state = fileState.get(sourcePath);
    const entries = sortByOrderThenId(section.entries, "entry_id");
    const generatedBlock = [
      section.start_marker,
      ...entries.map(renderEntry),
      section.end_marker,
    ].join(state.newline);

    const blockPattern = new RegExp(
      `${escapeRegExp(section.start_marker)}[\\s\\S]*?${escapeRegExp(section.end_marker)}`,
      "m",
    );
    if (!blockPattern.test(state.updated)) {
      fail(
        `unable to find markers for section ${section.section_id} in ${section.source_file}. ` +
          "Add both start and end markers before running this script.",
      );
    }

    state.updated = state.updated.replace(blockPattern, generatedBlock);
  }

  return fileState;
}

function writeOrCheck(fileState) {
  const changedFiles = [];

  for (const [filePath, state] of fileState.entries()) {
    if (state.original === state.updated) {
      continue;
    }
    changedFiles.push(normalizePath(filePath));
    if (!checkMode) {
      fs.writeFileSync(filePath, state.updated, "utf-8");
    }
  }

  if (checkMode && changedFiles.length > 0) {
    fail(
      `generated output is stale in: ${changedFiles.join(", ")}\n` +
        "Run: node ./scripts/sync_sources_menu_registry.mjs",
    );
  }

  if (changedFiles.length > 0) {
    console.log(`[sources-menu] updated: ${changedFiles.join(", ")}`);
    return;
  }

  if (checkMode) {
    console.log("[sources-menu] check passed.");
    return;
  }

  console.log("[sources-menu] registry validation passed; no changes.");
}

function main() {
  const schema = readJson(schemaPath);
  const registry = readJson(registryPath);
  validateRegistry(schema, registry);
  const fileState = applyGeneratedSections(registry);
  writeOrCheck(fileState);
}

try {
  main();
} catch (error) {
  console.error(`[sources-menu] ${error.message}`);
  process.exit(1);
}
