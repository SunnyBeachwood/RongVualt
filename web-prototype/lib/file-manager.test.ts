import { describe, expect, it } from 'vitest';
import {
  FileManagerError,
  clearSelection,
  createArchive,
  createFolder,
  createInitialState,
  deleteEntries,
  getChildren,
  getPath,
  goBack,
  goForward,
  goUp,
  navigateTo,
  renameEntry,
  toggleSelection,
  transferEntries,
} from './file-manager';

function namesIn(state: ReturnType<typeof createInitialState>, parentId: string): string[] {
  return getChildren(state, parentId).map((entry) => entry.name);
}

function expectManagerError(run: () => unknown, code: string) {
  try {
    run();
    throw new Error('Expected FileManagerError');
  } catch (error) {
    expect(error).toBeInstanceOf(FileManagerError);
    expect((error as FileManagerError).code).toBe(code);
  }
}

describe('file manager state', () => {
  it('starts with two independent realistic paths', () => {
    const state = createInitialState();
    expect(getPath(state, state.panes.left.currentDirectoryId)).toBe('/storage/emulated/0');
    expect(getPath(state, state.panes.right.currentDirectoryId)).toBe('/data/local/tmp');
    expect(namesIn(state, 'storage0')).toContain('Download');
    expect(namesIn(state, 'tmp')).toContain('workspace');
  });

  it('keeps each pane navigation and history independent', () => {
    let state = createInitialState();
    state = navigateTo(state, 'left', 'download');
    expect(getPath(state, state.panes.left.currentDirectoryId)).toBe('/storage/emulated/0/Download');
    expect(getPath(state, state.panes.right.currentDirectoryId)).toBe('/data/local/tmp');

    state = goBack(state, 'left');
    expect(state.panes.left.currentDirectoryId).toBe('storage0');
    state = goForward(state, 'left');
    expect(state.panes.left.currentDirectoryId).toBe('download');
    state = goUp(state, 'left');
    expect(state.panes.left.currentDirectoryId).toBe('storage0');
  });

  it('toggles and clears multi-selection in one pane', () => {
    let state = createInitialState();
    state = toggleSelection(state, 'left', 'download');
    state = toggleSelection(state, 'left', 'pictures');
    expect(state.panes.left.selectedIds).toEqual(['download', 'pictures']);
    expect(state.panes.right.selectedIds).toEqual([]);
    state = toggleSelection(state, 'left', 'download');
    expect(state.panes.left.selectedIds).toEqual(['pictures']);
    state = clearSelection(state, 'left');
    expect(state.panes.left.selectedIds).toEqual([]);
  });

  it('copies files to the other pane and generates conflict-safe names', () => {
    let state = createInitialState();
    state = transferEntries(state, 'left', ['trip-list'], 'copy');
    expect(namesIn(state, 'storage0')).toContain('旅行清单.md');
    expect(namesIn(state, 'tmp')).toContain('旅行清单.md');

    state = transferEntries(state, 'left', ['trip-list'], 'copy');
    expect(namesIn(state, 'tmp')).toContain('旅行清单 (副本).md');
  });

  it('copies a folder together with its descendants', () => {
    const state = transferEntries(createInitialState(), 'left', ['download'], 'copy');
    const copied = getChildren(state, 'tmp').find((entry) => entry.name === 'Download');
    expect(copied?.kind).toBe('folder');
    expect(copied && namesIn(state, copied.id)).toContain('RongVualt-1.3.0-arm64_v8.apk');
  });

  it('creates a conflict-safe ZIP beside the selected item', () => {
    let state = createInitialState();
    state = createArchive(state, 'left', 'trip-list');
    expect(namesIn(state, 'storage0')).toContain('旅行清单.zip');
    const archive = getChildren(state, 'storage0').find((entry) => entry.name === '旅行清单.zip');
    expect(archive).toMatchObject({ kind: 'file', fileType: 'archive' });

    state = createArchive(state, 'left', 'trip-list');
    expect(namesIn(state, 'storage0')).toContain('旅行清单 (副本).zip');
  });

  it('supports the archive formats shown in the compression menu', () => {
    const state = createArchive(createInitialState(), 'left', 'trip-list', '7z');
    expect(namesIn(state, 'storage0')).toContain('旅行清单.7z');
  });

  it('moves files instead of cloning them', () => {
    const state = transferEntries(createInitialState(), 'left', ['trip-list'], 'move');
    expect(namesIn(state, 'storage0')).not.toContain('旅行清单.md');
    expect(namesIn(state, 'tmp')).toContain('旅行清单.md');
    expect(state.entries['trip-list'].parentId).toBe('tmp');
  });

  it('rejects transfer into a selected folder descendant', () => {
    let state = createInitialState();
    state = navigateTo(state, 'right', 'android-data');
    expectManagerError(() => transferEntries(state, 'left', ['android'], 'move'), 'DESTINATION_INSIDE_SOURCE');
  });

  it('creates folders and validates duplicate or invalid names', () => {
    const initial = createInitialState();
    const created = createFolder(initial, 'right', '交付文件');
    expect(namesIn(created, 'tmp')).toContain('交付文件');
    expectManagerError(() => createFolder(created, 'right', '交付文件'), 'NAME_CONFLICT');
    expectManagerError(() => createFolder(created, 'right', '../escape'), 'INVALID_NAME');
  });

  it('renames one entry and clears the selection', () => {
    let state = createInitialState();
    state = toggleSelection(state, 'right', 'readme');
    state = renameEntry(state, 'right', 'readme', '说明.md');
    expect(state.entries.readme.name).toBe('说明.md');
    expect(state.panes.right.selectedIds).toEqual([]);
    expectManagerError(() => renameEntry(state, 'right', 'readme', 'release-notes.txt'), 'NAME_CONFLICT');
  });

  it('deletes selected folders recursively but protects an open directory', () => {
    let state = createInitialState();
    state = deleteEntries(state, 'left', ['download']);
    expect(state.entries.download).toBeUndefined();
    expect(state.entries['download-apk']).toBeUndefined();

    state = createInitialState();
    state = navigateTo(state, 'right', 'download');
    expectManagerError(() => deleteEntries(state, 'left', ['download']), 'DIRECTORY_IN_USE');
  });

  it('can reset back to an untouched initial state', () => {
    const changed = createFolder(createInitialState(), 'left', '临时测试');
    expect(namesIn(changed, 'storage0')).toContain('临时测试');
    const reset = createInitialState();
    expect(namesIn(reset, 'storage0')).not.toContain('临时测试');
    expect(reset.revision).toBe(0);
  });
});
