export type EntryKind = 'folder' | 'file';
export type PaneSide = 'left' | 'right';
export type TransferMode = 'copy' | 'move';
export type ArchiveFormat = 'zip' | '7z' | 'tar.gz';
export type FileType = 'archive' | 'image' | 'audio' | 'document' | 'apk' | 'generic';

export interface FileEntry {
  id: string;
  parentId: string | null;
  name: string;
  kind: EntryKind;
  modifiedAt: string;
  size?: number;
  fileType?: FileType;
  order: number;
}

export interface PaneState {
  currentDirectoryId: string;
  history: string[];
  future: string[];
  selectedIds: string[];
}

export interface FileManagerState {
  entries: Record<string, FileEntry>;
  panes: Record<PaneSide, PaneState>;
  activePane: PaneSide;
  revision: number;
}

export class FileManagerError extends Error {
  constructor(
    message: string,
    public readonly code: string,
  ) {
    super(message);
    this.name = 'FileManagerError';
  }
}

const NOW = '2026-09-05T21:00:00+08:00';

const SEED_ENTRIES: FileEntry[] = [
  { id: 'root', parentId: null, name: '', kind: 'folder', modifiedAt: NOW, order: 0 },
  { id: 'storage', parentId: 'root', name: 'storage', kind: 'folder', modifiedAt: '2026-08-31T16:42:00+08:00', order: 1 },
  { id: 'data', parentId: 'root', name: 'data', kind: 'folder', modifiedAt: '2026-09-03T15:35:00+08:00', order: 2 },
  { id: 'system', parentId: 'root', name: 'system', kind: 'folder', modifiedAt: '2026-08-11T12:00:00+08:00', order: 3 },
  { id: 'vendor', parentId: 'root', name: 'vendor', kind: 'folder', modifiedAt: '2026-08-11T12:00:00+08:00', order: 4 },
  { id: 'mnt', parentId: 'root', name: 'mnt', kind: 'folder', modifiedAt: '2026-08-31T16:42:00+08:00', order: 5 },

  { id: 'emulated', parentId: 'storage', name: 'emulated', kind: 'folder', modifiedAt: '2026-09-05T20:45:00+08:00', order: 1 },
  { id: 'self', parentId: 'storage', name: 'self', kind: 'folder', modifiedAt: '2026-08-31T16:42:00+08:00', order: 2 },
  { id: 'storage0', parentId: 'emulated', name: '0', kind: 'folder', modifiedAt: NOW, order: 1 },

  { id: 'download', parentId: 'storage0', name: 'Download', kind: 'folder', modifiedAt: '2026-09-05T18:32:00+08:00', order: 1 },
  { id: 'pictures', parentId: 'storage0', name: 'Pictures', kind: 'folder', modifiedAt: '2026-09-05T20:45:00+08:00', order: 2 },
  { id: 'documents', parentId: 'storage0', name: 'Documents', kind: 'folder', modifiedAt: '2026-09-04T21:15:00+08:00', order: 3 },
  { id: 'music', parentId: 'storage0', name: 'Music', kind: 'folder', modifiedAt: '2026-09-05T14:09:00+08:00', order: 4 },
  { id: 'movies', parentId: 'storage0', name: 'Movies', kind: 'folder', modifiedAt: '2026-09-05T14:09:00+08:00', order: 5 },
  { id: 'android', parentId: 'storage0', name: 'Android', kind: 'folder', modifiedAt: '2026-09-02T12:40:00+08:00', order: 6 },
  { id: 'temp-cn', parentId: 'storage0', name: '临时', kind: 'folder', modifiedAt: '2026-08-30T14:56:00+08:00', order: 7 },
  { id: 'trip-list', parentId: 'storage0', name: '旅行清单.md', kind: 'file', modifiedAt: '2026-09-05T19:52:00+08:00', size: 12_697, fileType: 'document', order: 8 },
  { id: 'vault-backup', parentId: 'storage0', name: 'vault-backup.zip', kind: 'file', modifiedAt: '2026-08-30T10:18:00+08:00', size: 299_892_736, fileType: 'archive', order: 9 },

  { id: 'download-archive', parentId: 'download', name: 'Archives', kind: 'folder', modifiedAt: '2026-09-05T17:04:00+08:00', order: 1 },
  { id: 'download-apk', parentId: 'download', name: 'RongVualt-1.3.0-arm64_v8.apk', kind: 'file', modifiedAt: '2026-09-05T18:32:00+08:00', size: 41_943_040, fileType: 'apk', order: 2 },
  { id: 'download-pdf', parentId: 'download', name: 'project-brief.pdf', kind: 'file', modifiedAt: '2026-09-04T09:12:00+08:00', size: 2_453_504, fileType: 'document', order: 3 },
  { id: 'download-7z', parentId: 'download-archive', name: 'materials.7z', kind: 'file', modifiedAt: '2026-09-03T11:30:00+08:00', size: 83_886_080, fileType: 'archive', order: 1 },

  { id: 'pictures-camera', parentId: 'pictures', name: 'Camera', kind: 'folder', modifiedAt: '2026-09-05T20:45:00+08:00', order: 1 },
  { id: 'pictures-screens', parentId: 'pictures', name: 'Screenshots', kind: 'folder', modifiedAt: '2026-09-05T21:00:00+08:00', order: 2 },
  { id: 'pictures-cover', parentId: 'pictures', name: 'cover.png', kind: 'file', modifiedAt: '2026-09-02T15:43:00+08:00', size: 892_928, fileType: 'image', order: 3 },
  { id: 'documents-notes', parentId: 'documents', name: '工作笔记.md', kind: 'file', modifiedAt: '2026-09-04T21:15:00+08:00', size: 28_672, fileType: 'document', order: 1 },
  { id: 'music-song', parentId: 'music', name: 'night-drive.flac', kind: 'file', modifiedAt: '2026-08-28T18:02:00+08:00', size: 36_700_160, fileType: 'audio', order: 1 },
  { id: 'android-data', parentId: 'android', name: 'data', kind: 'folder', modifiedAt: '2026-09-02T12:40:00+08:00', order: 1 },
  { id: 'android-media', parentId: 'android', name: 'media', kind: 'folder', modifiedAt: '2026-08-25T08:30:00+08:00', order: 2 },
  { id: 'android-obb', parentId: 'android', name: 'obb', kind: 'folder', modifiedAt: '2026-08-25T08:30:00+08:00', order: 3 },

  { id: 'local', parentId: 'data', name: 'local', kind: 'folder', modifiedAt: '2026-09-05T20:21:00+08:00', order: 1 },
  { id: 'data-misc', parentId: 'data', name: 'misc', kind: 'folder', modifiedAt: '2026-08-11T12:00:00+08:00', order: 2 },
  { id: 'data-system', parentId: 'data', name: 'system', kind: 'folder', modifiedAt: '2026-08-11T12:00:00+08:00', order: 3 },
  { id: 'tmp', parentId: 'local', name: 'tmp', kind: 'folder', modifiedAt: NOW, order: 1 },
  { id: 'local-state', parentId: 'local', name: 'state', kind: 'folder', modifiedAt: '2026-09-02T08:45:00+08:00', order: 2 },

  { id: 'cache', parentId: 'tmp', name: 'cache', kind: 'folder', modifiedAt: '2026-09-05T19:08:00+08:00', order: 1 },
  { id: 'exports', parentId: 'tmp', name: 'exports', kind: 'folder', modifiedAt: '2026-09-04T22:17:00+08:00', order: 2 },
  { id: 'unpacked', parentId: 'tmp', name: 'unpacked', kind: 'folder', modifiedAt: '2026-09-04T16:42:00+08:00', order: 3 },
  { id: 'workspace', parentId: 'tmp', name: 'workspace', kind: 'folder', modifiedAt: '2026-09-03T14:25:00+08:00', order: 4 },
  { id: 'readme', parentId: 'tmp', name: 'README.md', kind: 'file', modifiedAt: '2026-09-05T18:49:00+08:00', size: 8_806, fileType: 'document', order: 5 },
  { id: 'release-notes', parentId: 'tmp', name: 'release-notes.txt', kind: 'file', modifiedAt: '2026-09-04T20:17:00+08:00', size: 4_301, fileType: 'document', order: 6 },
  { id: 'cache-thumbs', parentId: 'cache', name: 'thumbnails', kind: 'folder', modifiedAt: '2026-09-05T19:08:00+08:00', order: 1 },
  { id: 'cache-session', parentId: 'cache', name: 'session.bin', kind: 'file', modifiedAt: '2026-09-05T19:07:00+08:00', size: 148_992, fileType: 'generic', order: 2 },
  { id: 'workspace-src', parentId: 'workspace', name: 'src', kind: 'folder', modifiedAt: '2026-09-03T14:25:00+08:00', order: 1 },
  { id: 'workspace-config', parentId: 'workspace', name: 'config.json', kind: 'file', modifiedAt: '2026-09-03T14:20:00+08:00', size: 2_048, fileType: 'document', order: 2 },
];

function cloneSeedEntries(): Record<string, FileEntry> {
  return Object.fromEntries(SEED_ENTRIES.map((entry) => [entry.id, { ...entry }]));
}

export function createInitialState(): FileManagerState {
  return {
    entries: cloneSeedEntries(),
    panes: {
      left: { currentDirectoryId: 'storage0', history: [], future: [], selectedIds: [] },
      right: { currentDirectoryId: 'tmp', history: [], future: [], selectedIds: [] },
    },
    activePane: 'left',
    revision: 0,
  };
}

export function otherPane(side: PaneSide): PaneSide {
  return side === 'left' ? 'right' : 'left';
}

export function getEntry(state: FileManagerState, id: string): FileEntry {
  const entry = state.entries[id];
  if (!entry) throw new FileManagerError('找不到指定条目', 'ENTRY_NOT_FOUND');
  return entry;
}

export function getChildren(state: FileManagerState, parentId: string): FileEntry[] {
  return Object.values(state.entries)
    .filter((entry) => entry.parentId === parentId)
    .sort((left, right) => {
      if (left.kind !== right.kind) return left.kind === 'folder' ? -1 : 1;
      return left.order - right.order || left.name.localeCompare(right.name, 'zh-CN');
    });
}

export function getPath(state: FileManagerState, directoryId: string): string {
  const parts: string[] = [];
  let entry: FileEntry | undefined = getEntry(state, directoryId);
  while (entry && entry.parentId !== null) {
    parts.unshift(entry.name);
    entry = state.entries[entry.parentId];
  }
  return `/${parts.join('/')}`;
}

export function isDescendantOrSelf(
  state: FileManagerState,
  candidateId: string,
  ancestorId: string,
): boolean {
  let current: FileEntry | undefined = state.entries[candidateId];
  while (current) {
    if (current.id === ancestorId) return true;
    current = current.parentId ? state.entries[current.parentId] : undefined;
  }
  return false;
}

function updatePane(
  state: FileManagerState,
  side: PaneSide,
  update: (pane: PaneState) => PaneState,
): FileManagerState {
  return {
    ...state,
    panes: { ...state.panes, [side]: update(state.panes[side]) },
    revision: state.revision + 1,
  };
}

export function activatePane(state: FileManagerState, side: PaneSide): FileManagerState {
  if (state.activePane === side) return state;
  return { ...state, activePane: side, revision: state.revision + 1 };
}

export function navigateTo(
  state: FileManagerState,
  side: PaneSide,
  directoryId: string,
): FileManagerState {
  const directory = getEntry(state, directoryId);
  if (directory.kind !== 'folder') throw new FileManagerError('该条目不是文件夹', 'NOT_A_DIRECTORY');
  const pane = state.panes[side];
  if (pane.currentDirectoryId === directoryId) return activatePane(state, side);
  return {
    ...state,
    activePane: side,
    panes: {
      ...state.panes,
      [side]: {
        currentDirectoryId: directoryId,
        history: [...pane.history, pane.currentDirectoryId],
        future: [],
        selectedIds: [],
      },
    },
    revision: state.revision + 1,
  };
}

export function goBack(state: FileManagerState, side: PaneSide): FileManagerState {
  const pane = state.panes[side];
  if (pane.history.length === 0) return state;
  const target = pane.history[pane.history.length - 1];
  return updatePane(state, side, () => ({
    currentDirectoryId: target,
    history: pane.history.slice(0, -1),
    future: [pane.currentDirectoryId, ...pane.future],
    selectedIds: [],
  }));
}

export function goForward(state: FileManagerState, side: PaneSide): FileManagerState {
  const pane = state.panes[side];
  if (pane.future.length === 0) return state;
  const [target, ...future] = pane.future;
  return updatePane(state, side, () => ({
    currentDirectoryId: target,
    history: [...pane.history, pane.currentDirectoryId],
    future,
    selectedIds: [],
  }));
}

export function goUp(state: FileManagerState, side: PaneSide): FileManagerState {
  const parentId = getEntry(state, state.panes[side].currentDirectoryId).parentId;
  return parentId ? navigateTo(state, side, parentId) : state;
}

export function toggleSelection(
  state: FileManagerState,
  side: PaneSide,
  entryId: string,
): FileManagerState {
  const pane = state.panes[side];
  const entry = getEntry(state, entryId);
  if (entry.parentId !== pane.currentDirectoryId) {
    throw new FileManagerError('只能选择当前目录中的条目', 'ENTRY_OUTSIDE_CURRENT_DIRECTORY');
  }
  const selected = new Set(pane.selectedIds);
  if (selected.has(entryId)) selected.delete(entryId);
  else selected.add(entryId);
  return {
    ...updatePane(state, side, () => ({ ...pane, selectedIds: [...selected] })),
    activePane: side,
  };
}

export function clearSelection(state: FileManagerState, side: PaneSide): FileManagerState {
  if (state.panes[side].selectedIds.length === 0) return state;
  return updatePane(state, side, (pane) => ({ ...pane, selectedIds: [] }));
}

function normalizeName(name: string): string {
  const normalized = name.trim();
  if (!normalized) throw new FileManagerError('名称不能为空', 'INVALID_NAME');
  if (normalized === '.' || normalized === '..' || normalized.includes('/') || normalized.includes('\0')) {
    throw new FileManagerError('名称不能包含“/”，也不能使用“.”或“..”', 'INVALID_NAME');
  }
  return normalized;
}

function nameExists(
  entries: Record<string, FileEntry>,
  parentId: string,
  name: string,
  exceptId?: string,
): boolean {
  return Object.values(entries).some(
    (entry) => entry.parentId === parentId && entry.id !== exceptId && entry.name === name,
  );
}

function nextOrder(entries: Record<string, FileEntry>, parentId: string): number {
  return Object.values(entries).reduce(
    (highest, entry) => (entry.parentId === parentId ? Math.max(highest, entry.order) : highest),
    0,
  ) + 1;
}

function generatedId(state: FileManagerState, index = 0): string {
  let candidate = `demo-${state.revision + 1}-${Object.keys(state.entries).length + index}`;
  let suffix = 2;
  while (state.entries[candidate]) candidate = `${candidate}-${suffix++}`;
  return candidate;
}

export function createFolder(
  state: FileManagerState,
  side: PaneSide,
  name: string,
): FileManagerState {
  const normalized = normalizeName(name);
  const parentId = state.panes[side].currentDirectoryId;
  if (nameExists(state.entries, parentId, normalized)) {
    throw new FileManagerError('当前目录中已有同名条目', 'NAME_CONFLICT');
  }
  const id = generatedId(state);
  return {
    ...state,
    entries: {
      ...state.entries,
      [id]: {
        id,
        parentId,
        name: normalized,
        kind: 'folder',
        modifiedAt: NOW,
        order: nextOrder(state.entries, parentId),
      },
    },
    activePane: side,
    revision: state.revision + 1,
  };
}

export function renameEntry(
  state: FileManagerState,
  side: PaneSide,
  entryId: string,
  name: string,
): FileManagerState {
  const normalized = normalizeName(name);
  const pane = state.panes[side];
  const entry = getEntry(state, entryId);
  if (entry.parentId !== pane.currentDirectoryId) {
    throw new FileManagerError('只能重命名当前目录中的条目', 'ENTRY_OUTSIDE_CURRENT_DIRECTORY');
  }
  if (nameExists(state.entries, pane.currentDirectoryId, normalized, entryId)) {
    throw new FileManagerError('当前目录中已有同名条目', 'NAME_CONFLICT');
  }
  return {
    ...state,
    entries: { ...state.entries, [entryId]: { ...entry, name: normalized, modifiedAt: NOW } },
    panes: { ...state.panes, [side]: { ...pane, selectedIds: [] } },
    revision: state.revision + 1,
  };
}

function collectSubtreeIds(state: FileManagerState, rootId: string, bucket: Set<string>): void {
  if (bucket.has(rootId)) return;
  bucket.add(rootId);
  for (const child of getChildren(state, rootId)) collectSubtreeIds(state, child.id, bucket);
}

export function deleteEntries(
  state: FileManagerState,
  side: PaneSide,
  requestedIds: string[],
): FileManagerState {
  if (requestedIds.length === 0) throw new FileManagerError('请先选择要删除的条目', 'NO_SELECTION');
  const currentId = state.panes[side].currentDirectoryId;
  const ids = [...new Set(requestedIds)];
  for (const id of ids) {
    const entry = getEntry(state, id);
    if (entry.parentId !== currentId) {
      throw new FileManagerError('只能删除当前目录中的条目', 'ENTRY_OUTSIDE_CURRENT_DIRECTORY');
    }
    for (const pane of Object.values(state.panes)) {
      if (isDescendantOrSelf(state, pane.currentDirectoryId, id)) {
        throw new FileManagerError('另一个窗格正在浏览该文件夹，无法删除', 'DIRECTORY_IN_USE');
      }
    }
  }
  const deletedIds = new Set<string>();
  ids.forEach((id) => collectSubtreeIds(state, id, deletedIds));
  const entries = { ...state.entries };
  deletedIds.forEach((id) => delete entries[id]);
  return {
    ...state,
    entries,
    panes: { ...state.panes, [side]: { ...state.panes[side], selectedIds: [] } },
    revision: state.revision + 1,
  };
}

function splitExtension(name: string): { stem: string; extension: string } {
  const dot = name.lastIndexOf('.');
  if (dot <= 0) return { stem: name, extension: '' };
  return { stem: name.slice(0, dot), extension: name.slice(dot) };
}

function uniqueCopyName(
  entries: Record<string, FileEntry>,
  parentId: string,
  originalName: string,
): string {
  if (!nameExists(entries, parentId, originalName)) return originalName;
  const { stem, extension } = splitExtension(originalName);
  let index = 1;
  while (true) {
    const suffix = index === 1 ? ' (副本)' : ` (副本 ${index})`;
    const candidate = `${stem}${suffix}${extension}`;
    if (!nameExists(entries, parentId, candidate)) return candidate;
    index += 1;
  }
}

function uniqueArchiveName(
  entries: Record<string, FileEntry>,
  parentId: string,
  originalName: string,
  extension: ArchiveFormat,
): string {
  const { stem } = splitExtension(originalName);
  const baseName = stem || originalName;
  const withExtension = (suffix = '') => `${baseName}${suffix}.${extension}`;
  if (!nameExists(entries, parentId, withExtension())) return withExtension();
  let index = 1;
  while (true) {
    const suffix = index === 1 ? ' (副本)' : ` (副本 ${index})`;
    const candidate = withExtension(suffix);
    if (!nameExists(entries, parentId, candidate)) return candidate;
    index += 1;
  }
}

function subtreeSize(state: FileManagerState, entryId: string): number {
  const entry = getEntry(state, entryId);
  return (entry.size ?? 0) + getChildren(state, entryId)
    .reduce((total, child) => total + subtreeSize(state, child.id), 0);
}

/** Creates a simulated ZIP beside the source entry; no browser files are read. */
export function createArchive(
  state: FileManagerState,
  side: PaneSide,
  entryId: string,
  format: ArchiveFormat = 'zip',
): FileManagerState {
  const pane = state.panes[side];
  const source = getEntry(state, entryId);
  if (source.parentId !== pane.currentDirectoryId) {
    throw new FileManagerError('只能压缩当前目录中的条目', 'ENTRY_OUTSIDE_CURRENT_DIRECTORY');
  }
  const id = generatedId(state);
  const size = Math.max(1_024, Math.round(subtreeSize(state, entryId) * 0.65));
  return {
    ...state,
    entries: {
      ...state.entries,
      [id]: {
        id,
        parentId: source.parentId,
        name: uniqueArchiveName(state.entries, source.parentId, source.name, format),
        kind: 'file',
        fileType: 'archive',
        size,
        modifiedAt: NOW,
        order: nextOrder(state.entries, source.parentId),
      },
    },
    activePane: side,
    revision: state.revision + 1,
  };
}

function cloneSubtree(
  state: FileManagerState,
  sourceId: string,
  destinationParentId: string,
  destinationName: string,
  entries: Record<string, FileEntry>,
  allocateId: () => string,
): void {
  const source = getEntry(state, sourceId);
  const newId = allocateId();
  entries[newId] = {
    ...source,
    id: newId,
    parentId: destinationParentId,
    name: destinationName,
    modifiedAt: NOW,
    order: nextOrder(entries, destinationParentId),
  };
  if (source.kind === 'folder') {
    for (const child of getChildren(state, source.id)) {
      cloneSubtree(state, child.id, newId, child.name, entries, allocateId);
    }
  }
}

export function transferEntries(
  state: FileManagerState,
  sourceSide: PaneSide,
  requestedIds: string[],
  mode: TransferMode,
): FileManagerState {
  if (requestedIds.length === 0) throw new FileManagerError('请先选择要传送的条目', 'NO_SELECTION');
  const destinationSide = otherPane(sourceSide);
  const sourceDirectoryId = state.panes[sourceSide].currentDirectoryId;
  const destinationDirectoryId = state.panes[destinationSide].currentDirectoryId;
  if (sourceDirectoryId === destinationDirectoryId) {
    throw new FileManagerError('两个窗格位于同一目录，无需传送', 'SAME_DIRECTORY');
  }
  const ids = [...new Set(requestedIds)];
  for (const id of ids) {
    const entry = getEntry(state, id);
    if (entry.parentId !== sourceDirectoryId) {
      throw new FileManagerError('只能传送来源窗格当前目录中的条目', 'ENTRY_OUTSIDE_CURRENT_DIRECTORY');
    }
    if (entry.kind === 'folder' && isDescendantOrSelf(state, destinationDirectoryId, entry.id)) {
      throw new FileManagerError('不能将文件夹传送到自身或其子目录', 'DESTINATION_INSIDE_SOURCE');
    }
  }

  const entries = { ...state.entries };
  let allocationIndex = 0;
  const allocateId = () => {
    let id = generatedId(state, allocationIndex++);
    while (entries[id]) id = `${id}-${allocationIndex++}`;
    return id;
  };

  for (const id of ids) {
    const source = entries[id];
    const destinationName = uniqueCopyName(entries, destinationDirectoryId, source.name);
    if (mode === 'copy') {
      cloneSubtree(state, id, destinationDirectoryId, destinationName, entries, allocateId);
    } else {
      entries[id] = {
        ...source,
        parentId: destinationDirectoryId,
        name: destinationName,
        modifiedAt: NOW,
        order: nextOrder(entries, destinationDirectoryId),
      };
    }
  }

  return {
    ...state,
    entries,
    panes: {
      ...state.panes,
      [sourceSide]: { ...state.panes[sourceSide], selectedIds: [] },
    },
    activePane: sourceSide,
    revision: state.revision + 1,
  };
}
