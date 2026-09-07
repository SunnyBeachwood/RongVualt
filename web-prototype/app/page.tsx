'use client';

import {
  type FormEvent,
  type PointerEvent as ReactPointerEvent,
  useCallback,
  useEffect,
  useRef,
  useState,
} from 'react';
import { flushSync } from 'react-dom';
import {
  ArrowLeft,
  ArrowRight,
  ArrowUp,
  Archive,
  BookmarkCheck,
  BookmarkPlus,
  Check,
  ChevronsLeftRight,
  ClipboardCopy,
  Copy,
  File,
  FileArchive,
  FileAudio,
  FileImage,
  FileText,
  Folder,
  FolderPlus,
  HardDrive,
  Info,
  MoreVertical,
  MoveRight,
  Pencil,
  RefreshCcw,
  Share2,
  SquareCheckBig,
  Trash2,
  X,
} from 'lucide-react';
import {
  AlertDialog,
  AlertDialogCancel,
  AlertDialogContent,
  AlertDialogDescription,
  AlertDialogFooter,
  AlertDialogHeader,
  AlertDialogTitle,
} from '@/components/ui/alert-dialog';
import { Button } from '@/components/ui/button';
import {
  Dialog,
  DialogContent,
  DialogDescription,
  DialogFooter,
  DialogHeader,
  DialogTitle,
} from '@/components/ui/dialog';
import {
  DropdownMenu,
  DropdownMenuContent,
  DropdownMenuGroup,
  DropdownMenuItem,
  DropdownMenuLabel,
  DropdownMenuSeparator,
  DropdownMenuTrigger,
} from '@/components/ui/dropdown-menu';
import { Input } from '@/components/ui/input';
import { Toaster, toast } from '@/components/ui/toast';
import {
  type FileEntry,
  type FileManagerState,
  type ArchiveFormat,
  type PaneSide,
  type TransferMode,
  FileManagerError,
  activatePane,
  clearSelection,
  createArchive,
  createFolder,
  createInitialState,
  deleteEntries,
  getChildren,
  getEntry,
  getPath,
  goBack,
  goForward,
  goUp,
  navigateTo,
  otherPane,
  renameEntry,
  toggleSelection,
  transferEntries,
} from '@/lib/file-manager';

interface WebMcpTool {
  name: string;
  title?: string;
  description: string;
  inputSchema: Record<string, unknown>;
  annotations?: { readOnlyHint?: boolean; untrustedContentHint?: boolean };
  execute(input: unknown): unknown | Promise<unknown>;
}

declare global {
  interface Document {
    readonly modelContext?: {
      registerTool(tool: WebMcpTool, options?: { signal?: AbortSignal }): void | Promise<void>;
    };
  }
}

type NameDialogState =
  | { mode: 'create'; side: PaneSide }
  | { mode: 'rename'; side: PaneSide; entryId: string }
  | null;

type EntryTarget = { side: PaneSide; entryId: string };
type DeleteRequest = { side: PaneSide; entryIds: string[] };

function formatDate(value: string): string {
  const date = value.slice(0, 10);
  const time = value.slice(11, 16);
  if (date === '2026-09-05') return `今天 ${time}`;
  if (date === '2026-09-04') return `昨天 ${time}`;
  const [, month, day] = date.split('-');
  return `${month}月${day}日 ${time}`;
}

function formatSize(bytes: number): string {
  if (bytes < 1024) return `${bytes} B`;
  if (bytes < 1024 ** 2) return `${(bytes / 1024).toFixed(bytes < 10 * 1024 ? 1 : 0)} KB`;
  if (bytes < 1024 ** 3) return `${(bytes / 1024 ** 2).toFixed(bytes < 10 * 1024 ** 2 ? 1 : 0)} MB`;
  return `${(bytes / 1024 ** 3).toFixed(1)} GB`;
}

function entryMeta(entry: FileEntry): string {
  const date = formatDate(entry.modifiedAt);
  return entry.kind === 'file' && entry.size !== undefined ? `${formatSize(entry.size)} · ${date}` : date;
}

function FileIcon({ entry }: { entry: FileEntry }) {
  if (entry.kind === 'folder') return <Folder aria-hidden="true" />;
  if (entry.fileType === 'archive') return <FileArchive aria-hidden="true" />;
  if (entry.fileType === 'image') return <FileImage aria-hidden="true" />;
  if (entry.fileType === 'audio') return <FileAudio aria-hidden="true" />;
  if (entry.fileType === 'document') return <FileText aria-hidden="true" />;
  return <File aria-hidden="true" />;
}

function FileRow({
  entry,
  selected,
  onActivate,
  onOpen,
  onToggle,
  onContextMenuOpen,
  onClearSelection,
}: {
  entry: FileEntry;
  selected: boolean;
  onActivate: () => void;
  onOpen: () => void;
  onToggle: () => void;
  onContextMenuOpen: () => void;
  onClearSelection: () => void;
}) {
  const timerRef = useRef<ReturnType<typeof setTimeout> | null>(null);
  const longPressedRef = useRef(false);

  const cancelLongPress = () => {
    if (timerRef.current) clearTimeout(timerRef.current);
    timerRef.current = null;
  };

  const startLongPress = (event: ReactPointerEvent<HTMLButtonElement>) => {
    if (event.pointerType === 'mouse' && event.button !== 0) return;
    onActivate();
    longPressedRef.current = false;
    cancelLongPress();
    timerRef.current = setTimeout(() => {
      longPressedRef.current = true;
      onContextMenuOpen();
    }, 460);
  };

  return (
    <button
      type="button"
      className="file-row"
      data-selected={selected}
      aria-pressed={selected}
      aria-label={`${entry.kind === 'folder' ? '文件夹' : '文件'} ${entry.name}${selected ? '，已选择' : ''}`}
      onPointerDown={startLongPress}
      onPointerUp={cancelLongPress}
      onPointerCancel={cancelLongPress}
      onPointerLeave={cancelLongPress}
      onContextMenu={(event) => {
        event.preventDefault();
        cancelLongPress();
        onContextMenuOpen();
      }}
      onClick={() => {
        if (longPressedRef.current) {
          longPressedRef.current = false;
          return;
        }
        onActivate();
        onOpen();
      }}
      onKeyDown={(event) => {
        if (event.key === ' ') {
          event.preventDefault();
          onActivate();
          onToggle();
        }
        if (event.key === 'Escape') {
          event.preventDefault();
          onClearSelection();
        }
      }}
    >
      <span className={`file-icon file-icon--${entry.kind === 'folder' ? 'folder' : entry.fileType ?? 'generic'}`}>
        <FileIcon entry={entry} />
        <span className="selection-check" aria-hidden="true"><Check /></span>
      </span>
      <span className="file-copy">
        <span className="file-name">{entry.name}</span>
        <span className="file-meta">{entryMeta(entry)}</span>
      </span>
    </button>
  );
}

function FilePane({
  state,
  side,
  onActivate,
  onNavigate,
  onOpenFile,
  onToggle,
  onContextMenuOpen,
  onClearSelection,
}: {
  state: FileManagerState;
  side: PaneSide;
  onActivate: () => void;
  onNavigate: (entryId: string) => void;
  onOpenFile: (entryId: string) => void;
  onToggle: (entryId: string) => void;
  onContextMenuOpen: (entryId: string) => void;
  onClearSelection: () => void;
}) {
  const pane = state.panes[side];
  const entries = getChildren(state, pane.currentDirectoryId);
  const selected = new Set(pane.selectedIds);
  const listRef = useRef<HTMLDivElement>(null);
  const scrollPositions = useRef<Record<string, number>>({});

  useEffect(() => {
    const list = listRef.current;
    if (!list) return;
    list.scrollTop = scrollPositions.current[pane.currentDirectoryId] ?? 0;
  }, [pane.currentDirectoryId]);

  return (
    <section
      className="file-pane"
      data-active={state.activePane === side}
      aria-label={`${side === 'left' ? '左侧' : '右侧'}文件窗格`}
      onPointerDown={onActivate}
    >
      <header className="pane-header" onClick={onActivate}>
        <div className="pane-label">
          <span>{side === 'left' ? '左侧窗格' : '右侧窗格'}</span>
          {state.activePane === side && <strong>当前</strong>}
          {pane.selectedIds.length > 0 && <em>{pane.selectedIds.length} 项</em>}
        </div>
        <div className="pane-path" title={getPath(state, pane.currentDirectoryId)}>
          {getPath(state, pane.currentDirectoryId)}
        </div>
        <div className="pane-summary">
          {entries.filter((entry) => entry.kind === 'folder').length} 个文件夹 · {entries.filter((entry) => entry.kind === 'file').length} 个文件
        </div>
      </header>

      <div
        className="file-list"
        ref={listRef}
        onScroll={(event) => {
          scrollPositions.current[pane.currentDirectoryId] = event.currentTarget.scrollTop;
        }}
      >
        {entries.length === 0 ? (
          <div className="empty-folder">
            <Folder aria-hidden="true" />
            <strong>此文件夹为空</strong>
            <span>可使用底部“新建”添加文件夹</span>
          </div>
        ) : entries.map((entry) => (
          <FileRow
            key={entry.id}
            entry={entry}
            selected={selected.has(entry.id)}
            onActivate={onActivate}
            onOpen={() => entry.kind === 'folder' ? onNavigate(entry.id) : onOpenFile(entry.id)}
            onToggle={() => onToggle(entry.id)}
            onContextMenuOpen={() => onContextMenuOpen(entry.id)}
            onClearSelection={onClearSelection}
          />
        ))}
      </div>
    </section>
  );
}

function stateSnapshot(state: FileManagerState) {
  const paneSnapshot = (side: PaneSide) => {
    const pane = state.panes[side];
    return {
      path: getPath(state, pane.currentDirectoryId),
      selectedIds: pane.selectedIds,
      entries: getChildren(state, pane.currentDirectoryId).map(({ id, name, kind, size }) => ({ id, name, kind, size })),
    };
  };
  return { activePane: state.activePane, left: paneSnapshot('left'), right: paneSnapshot('right') };
}

function parseTransferInput(input: unknown): { sourcePane: PaneSide; entryIds: string[]; mode: TransferMode } {
  if (!input || typeof input !== 'object') throw new FileManagerError('输入必须是对象', 'INVALID_TOOL_INPUT');
  const value = input as Record<string, unknown>;
  if (value.sourcePane !== 'left' && value.sourcePane !== 'right') {
    throw new FileManagerError('sourcePane 必须是 left 或 right', 'INVALID_TOOL_INPUT');
  }
  if (value.mode !== 'copy' && value.mode !== 'move') {
    throw new FileManagerError('mode 必须是 copy 或 move', 'INVALID_TOOL_INPUT');
  }
  if (!Array.isArray(value.entryIds) || value.entryIds.length === 0 || value.entryIds.some((id) => typeof id !== 'string')) {
    throw new FileManagerError('entryIds 必须是非空字符串数组', 'INVALID_TOOL_INPUT');
  }
  return { sourcePane: value.sourcePane, entryIds: value.entryIds as string[], mode: value.mode };
}

export default function Home() {
  const [manager, setManager] = useState<FileManagerState>(() => createInitialState());
  const managerRef = useRef(manager);
  const [nameDialog, setNameDialog] = useState<NameDialogState>(null);
  const [nameValue, setNameValue] = useState('');
  const [nameError, setNameError] = useState('');
  const [contextTarget, setContextTarget] = useState<EntryTarget | null>(null);
  const [archiveTargetMenu, setArchiveTargetMenu] = useState<EntryTarget | null>(null);
  const [openedFile, setOpenedFile] = useState<EntryTarget | null>(null);
  const [deleteRequest, setDeleteRequest] = useState<DeleteRequest | null>(null);
  const [bookmarkedIds, setBookmarkedIds] = useState<string[]>([]);

  const commit = useCallback((operation: (state: FileManagerState) => FileManagerState) => {
    const next = operation(managerRef.current);
    managerRef.current = next;
    setManager(next);
    return next;
  }, []);

  const notifyError = (error: unknown) => {
    toast.add({
      title: '操作未完成',
      description: error instanceof Error ? error.message : '发生未知错误',
      type: 'error',
      priority: 'high',
      timeout: 3800,
    });
  };

  const tryCommit = useCallback((
    operation: (state: FileManagerState) => FileManagerState,
    success?: { title: string; description?: string },
  ) => {
    try {
      const next = commit(operation);
      if (success) toast.add({ ...success, type: 'success', timeout: 2800 });
      return next;
    } catch (error) {
      notifyError(error);
      return null;
    }
  }, [commit]);

  useEffect(() => {
    const context = document.modelContext;
    if (!context?.registerTool) return;
    const lifecycle = new AbortController();

    const registrations = [
      context.registerTool({
        name: 'read_file_manager_state',
        title: '读取双栏文件状态',
        description: '读取当前双栏路径、可见条目、活动窗格和选择状态。不会更改演示数据。',
        inputSchema: { type: 'object', properties: {}, additionalProperties: false },
        annotations: { readOnlyHint: true, untrustedContentHint: true },
        execute() {
          return stateSnapshot(managerRef.current);
        },
      }, { signal: lifecycle.signal }),
      context.registerTool({
        name: 'transfer_entries',
        title: '在双栏之间传送条目',
        description: '将来源窗格当前目录中的一个或多个条目复制或移动到另一窗格当前目录。',
        inputSchema: {
          type: 'object',
          properties: {
            sourcePane: { type: 'string', enum: ['left', 'right'] },
            entryIds: { type: 'array', minItems: 1, uniqueItems: true, items: { type: 'string' } },
            mode: { type: 'string', enum: ['copy', 'move'] },
          },
          required: ['sourcePane', 'entryIds', 'mode'],
          additionalProperties: false,
        },
        annotations: { readOnlyHint: false, untrustedContentHint: true },
        execute(input) {
          const { sourcePane, entryIds, mode } = parseTransferInput(input);
          const next = transferEntries(managerRef.current, sourcePane, entryIds, mode);
          managerRef.current = next;
          flushSync(() => setManager(next));
          toast.add({
            title: mode === 'copy' ? '复制完成' : '移动完成',
            description: `${entryIds.length} 项已传送到${otherPane(sourcePane) === 'left' ? '左侧' : '右侧'}窗格`,
            type: 'success',
            timeout: 2800,
          });
          return { ok: true, mode, count: entryIds.length, destinationPath: getPath(next, next.panes[otherPane(sourcePane)].currentDirectoryId) };
        },
      }, { signal: lifecycle.signal }),
    ];
    registrations.forEach((registration) => void Promise.resolve(registration).catch(() => undefined));
    return () => lifecycle.abort();
  }, []);

  useEffect(() => {
    const handleEscape = (event: KeyboardEvent) => {
      if (event.key !== 'Escape' || nameDialog || contextTarget || openedFile || deleteRequest) return;
      const side = managerRef.current.activePane;
      if (managerRef.current.panes[side].selectedIds.length > 0) commit((state) => clearSelection(state, side));
    };
    window.addEventListener('keydown', handleEscape);
    return () => window.removeEventListener('keydown', handleEscape);
  }, [commit, contextTarget, deleteRequest, nameDialog, openedFile]);

  const activeSide = manager.activePane;
  const active = manager.panes[activeSide];
  const selectedCount = active.selectedIds.length;
  const activePath = getPath(manager, active.currentDirectoryId);
  const parentId = getEntry(manager, active.currentDirectoryId).parentId;
  const contextEntry = contextTarget ? manager.entries[contextTarget.entryId] : undefined;
  const openedEntry = openedFile ? manager.entries[openedFile.entryId] : undefined;

  const openCreate = () => {
    setNameDialog({ mode: 'create', side: activeSide });
    setNameValue('新建文件夹');
    setNameError('');
  };

  const openRename = () => {
    if (selectedCount !== 1) return;
    const entryId = active.selectedIds[0];
    setNameDialog({ mode: 'rename', side: activeSide, entryId });
    setNameValue(getEntry(manager, entryId).name);
    setNameError('');
  };

  const openTargetRename = (target: EntryTarget) => {
    setNameDialog({ mode: 'rename', side: target.side, entryId: target.entryId });
    setNameValue(getEntry(managerRef.current, target.entryId).name);
    setNameError('');
    setContextTarget(null);
  };

  const openEntryMenu = (target: EntryTarget) => {
    tryCommit((state) => activatePane(state, target.side));
    setContextTarget(target);
  };

  const openFile = (target: EntryTarget) => {
    tryCommit((state) => activatePane(state, target.side));
    setOpenedFile(target);
  };

  const submitName = (event: FormEvent) => {
    event.preventDefault();
    if (!nameDialog) return;
    try {
      if (nameDialog.mode === 'create') {
        commit((state) => createFolder(state, nameDialog.side, nameValue));
        toast.add({ title: '文件夹已创建', description: nameValue.trim(), type: 'success', timeout: 2600 });
      } else {
        commit((state) => renameEntry(state, nameDialog.side, nameDialog.entryId, nameValue));
        toast.add({ title: '重命名完成', description: nameValue.trim(), type: 'success', timeout: 2600 });
      }
      setNameDialog(null);
      setNameError('');
    } catch (error) {
      setNameError(error instanceof Error ? error.message : '名称无效');
    }
  };

  const transfer = (mode: TransferMode) => {
    const destination = otherPane(activeSide);
    tryCommit(
      (state) => transferEntries(state, activeSide, active.selectedIds, mode),
      {
        title: mode === 'copy' ? '复制完成' : '移动完成',
        description: `${selectedCount} 项已传送到${destination === 'left' ? '左侧' : '右侧'}窗格`,
      },
    );
  };

  const confirmDelete = () => {
    if (!deleteRequest) return;
    const count = deleteRequest.entryIds.length;
    const result = tryCommit(
      (state) => deleteEntries(state, deleteRequest.side, deleteRequest.entryIds),
      { title: '已删除', description: `${count} 项已从演示数据中移除` },
    );
    if (result) setDeleteRequest(null);
  };

  const deleteSelected = () => {
    if (selectedCount === 0) return;
    setDeleteRequest({ side: activeSide, entryIds: active.selectedIds });
  };

  const transferTarget = (target: EntryTarget, mode: TransferMode) => {
    const destination = otherPane(target.side);
    const result = tryCommit(
      (state) => transferEntries(state, target.side, [target.entryId], mode),
      {
        title: mode === 'copy' ? '复制完成' : '移动完成',
        description: `已传送到${destination === 'left' ? '左侧' : '右侧'}窗格`,
      },
    );
    if (result) setContextTarget(null);
  };

  const archiveTarget = (target: EntryTarget, format: ArchiveFormat) => {
    const result = tryCommit(
      (state) => createArchive(state, target.side, target.entryId, format),
      { title: '压缩完成', description: `已在当前目录创建演示 ${format.toUpperCase()} 文件` },
    );
    if (result) setArchiveTargetMenu(null);
  };

  const targetPath = (target: EntryTarget): string => {
    const state = managerRef.current;
    const entry = getEntry(state, target.entryId);
    const directory = entry.parentId ? getPath(state, entry.parentId) : '';
    return `${directory}/${entry.name}`.replace(/^\/{2,}/, '/');
  };

  const copyTargetPath = async (target: EntryTarget) => {
    const path = targetPath(target);
    try {
      if (!navigator.clipboard?.writeText) throw new Error('Clipboard unavailable');
      await navigator.clipboard.writeText(path);
      toast.add({ title: '路径已复制', description: path, type: 'success', timeout: 3000 });
    } catch {
      toast.add({ title: '路径', description: path, type: 'info', timeout: 4200 });
    }
    setContextTarget(null);
  };

  const shareTarget = async (target: EntryTarget) => {
    const entry = getEntry(managerRef.current, target.entryId);
    const path = targetPath(target);
    try {
      if (navigator.share) {
        await navigator.share({ title: entry.name, text: path });
        toast.add({ title: '已调用系统分享', description: entry.name, type: 'success', timeout: 2800 });
      } else {
        if (!navigator.clipboard?.writeText) throw new Error('Clipboard unavailable');
        await navigator.clipboard.writeText(path);
        toast.add({ title: '已复制分享路径', description: path, type: 'success', timeout: 3000 });
      }
    } catch (error) {
      if (!(error instanceof DOMException && error.name === 'AbortError')) {
        toast.add({ title: '分享未完成', description: path, type: 'info', timeout: 3800 });
      }
    }
    setContextTarget(null);
  };

  const toggleTargetBookmark = (target: EntryTarget) => {
    const isBookmarked = bookmarkedIds.includes(target.entryId);
    setBookmarkedIds((ids) => isBookmarked ? ids.filter((id) => id !== target.entryId) : [...ids, target.entryId]);
    toast.add({ title: isBookmarked ? '已取消书签' : '已添加书签', description: getEntry(managerRef.current, target.entryId).name, type: 'success', timeout: 2400 });
    setContextTarget(null);
  };

  const selectTarget = (target: EntryTarget) => {
    const result = tryCommit((state) => toggleSelection(state, target.side, target.entryId));
    if (result) setContextTarget(null);
  };

  const showTargetProperties = (target: EntryTarget) => {
    const entry = getEntry(managerRef.current, target.entryId);
    const directory = entry.parentId ? getPath(managerRef.current, entry.parentId) : '/';
    toast.add({
      title: entry.name,
      description: `${entry.kind === 'folder' ? '文件夹' : '文件'} · ${entryMeta(entry)} · ${directory}`,
      type: 'info',
      timeout: 4200,
    });
    setContextTarget(null);
  };

  const resetDemo = () => {
    const next = createInitialState();
    managerRef.current = next;
    setManager(next);
    setNameDialog(null);
    setContextTarget(null);
    setArchiveTargetMenu(null);
    setOpenedFile(null);
    setDeleteRequest(null);
    setBookmarkedIds([]);
    toast.add({ title: '演示数据已重置', type: 'info', timeout: 2400 });
  };

  return (
    <Toaster>
      <main className="file-manager-shell">
        <header className="app-bar">
          <div className="app-mark"><HardDrive aria-hidden="true" /></div>
          <div className="app-title">
            <h1>双栏文件管理器</h1>
            <p>
              <span>演示数据</span>
              {selectedCount > 0 ? `${activeSide === 'left' ? '左侧' : '右侧'}已选 ${selectedCount} 项` : `当前：${activePath}`}
            </p>
          </div>
          <DropdownMenu>
            <DropdownMenuTrigger render={<Button variant="ghost" size="icon" className="top-action" aria-label="更多选项" />}>
              <MoreVertical />
            </DropdownMenuTrigger>
            <DropdownMenuContent align="end" sideOffset={8} className="top-menu">
              <DropdownMenuGroup>
                <DropdownMenuLabel>演示选项</DropdownMenuLabel>
                <DropdownMenuSeparator />
                <DropdownMenuItem onClick={resetDemo}>
                  <RefreshCcw /> 重置演示数据
                </DropdownMenuItem>
              </DropdownMenuGroup>
            </DropdownMenuContent>
          </DropdownMenu>
        </header>

        <div className="dual-pane">
          {(['left', 'right'] as const).map((side) => (
            <FilePane
              key={side}
              state={manager}
              side={side}
              onActivate={() => tryCommit((state) => activatePane(state, side))}
              onNavigate={(entryId) => tryCommit((state) => navigateTo(state, side, entryId))}
              onOpenFile={(entryId) => openFile({ side, entryId })}
              onToggle={(entryId) => tryCommit((state) => toggleSelection(state, side, entryId))}
              onContextMenuOpen={(entryId) => openEntryMenu({ side, entryId })}
              onClearSelection={() => tryCommit((state) => clearSelection(state, side))}
            />
          ))}
        </div>

        {selectedCount > 0 ? (
          <nav className="action-dock selection-dock" aria-label="所选文件操作">
            <Button variant="ghost" onClick={() => transfer('copy')}><Copy /><span>复制到</span></Button>
            <Button variant="ghost" onClick={() => transfer('move')}><MoveRight /><span>移动到</span></Button>
            <Button variant="ghost" onClick={openRename} disabled={selectedCount !== 1}><Pencil /><span>重命名</span></Button>
            <Button variant="ghost" className="delete-action" onClick={deleteSelected}><Trash2 /><span>删除</span></Button>
            <Button variant="ghost" onClick={() => tryCommit((state) => clearSelection(state, activeSide))}><X /><span>取消</span></Button>
          </nav>
        ) : (
          <nav className="action-dock" aria-label="目录操作">
            <Button variant="ghost" disabled={active.history.length === 0} onClick={() => tryCommit((state) => goBack(state, activeSide))}><ArrowLeft /><span>后退</span></Button>
            <Button variant="ghost" disabled={active.future.length === 0} onClick={() => tryCommit((state) => goForward(state, activeSide))}><ArrowRight /><span>前进</span></Button>
            <Button variant="ghost" disabled={!parentId} onClick={() => tryCommit((state) => goUp(state, activeSide))}><ArrowUp /><span>上级</span></Button>
            <Button variant="ghost" onClick={openCreate}><FolderPlus /><span>新建</span></Button>
            <Button variant="ghost" onClick={() => tryCommit((state) => activatePane(state, otherPane(activeSide)))}><ChevronsLeftRight /><span>切换</span></Button>
          </nav>
        )}
      </main>

      <Dialog open={contextTarget !== null} onOpenChange={(open) => { if (!open) setContextTarget(null); }}>
        <DialogContent className="entry-menu-dialog" showCloseButton={false}>
          {contextEntry && contextTarget && (
            <>
              <DialogHeader className="entry-menu-heading">
                <DialogTitle>{contextEntry.name}</DialogTitle>
                <DialogDescription>
                  长按操作 · {contextEntry.kind === 'folder' ? '文件夹' : '文件'} · {entryMeta(contextEntry)}
                </DialogDescription>
              </DialogHeader>
              <div className="entry-menu-actions" aria-label={`${contextEntry.name} 的操作`}>
                <Button variant="ghost" onClick={() => transferTarget(contextTarget, 'copy')}>
                  <Copy /><span>复制到另一窗格</span>
                </Button>
                <Button variant="ghost" onClick={() => transferTarget(contextTarget, 'move')}>
                  <MoveRight /><span>移动到另一窗格</span>
                </Button>
                <Button variant="ghost" onClick={() => openTargetRename(contextTarget)}>
                  <Pencil /><span>重命名</span>
                </Button>
                <Button variant="ghost" onClick={() => { setArchiveTargetMenu(contextTarget); setContextTarget(null); }}>
                  <Archive /><span>压缩</span>
                </Button>
                <Button variant="ghost" onClick={() => void copyTargetPath(contextTarget)}>
                  <ClipboardCopy /><span>复制路径</span>
                </Button>
                <Button variant="ghost" onClick={() => void shareTarget(contextTarget)}>
                  <Share2 /><span>分享</span>
                </Button>
                <Button variant="ghost" onClick={() => toggleTargetBookmark(contextTarget)}>
                  {bookmarkedIds.includes(contextTarget.entryId) ? <BookmarkCheck /> : <BookmarkPlus />}
                  <span>{bookmarkedIds.includes(contextTarget.entryId) ? '取消书签' : '添加书签'}</span>
                </Button>
                <Button variant="ghost" onClick={() => selectTarget(contextTarget)}>
                  <SquareCheckBig /><span>选择</span>
                </Button>
                <Button variant="ghost" onClick={() => showTargetProperties(contextTarget)}>
                  <Info /><span>属性</span>
                </Button>
                <Button variant="ghost" className="entry-menu-delete" onClick={() => {
                  setDeleteRequest({ side: contextTarget.side, entryIds: [contextTarget.entryId] });
                  setContextTarget(null);
                }}>
                  <Trash2 /><span>删除</span>
                </Button>
                <Button variant="ghost" className="entry-menu-cancel" onClick={() => setContextTarget(null)}>
                  <X /><span>取消</span>
                </Button>
              </div>
            </>
          )}
        </DialogContent>
      </Dialog>

      <Dialog open={archiveTargetMenu !== null} onOpenChange={(open) => { if (!open) setArchiveTargetMenu(null); }}>
        <DialogContent className="archive-menu-dialog" showCloseButton={false}>
          {archiveTargetMenu && manager.entries[archiveTargetMenu.entryId] && (
            <>
              <DialogHeader>
                <DialogTitle>压缩</DialogTitle>
                <DialogDescription>选择 {manager.entries[archiveTargetMenu.entryId].name} 的压缩格式</DialogDescription>
              </DialogHeader>
              <div className="archive-format-actions" aria-label="选择压缩格式">
                <Button variant="ghost" onClick={() => archiveTarget(archiveTargetMenu, 'zip')}>
                  <FileArchive /><span><strong>ZIP</strong><small>兼容性最好</small></span>
                </Button>
                <Button variant="ghost" onClick={() => archiveTarget(archiveTargetMenu, '7z')}>
                  <Archive /><span><strong>7Z</strong><small>较高压缩率</small></span>
                </Button>
                <Button variant="ghost" onClick={() => archiveTarget(archiveTargetMenu, 'tar.gz')}>
                  <FileArchive /><span><strong>TAR.GZ</strong><small>适合目录打包</small></span>
                </Button>
              </div>
              <DialogFooter className="archive-menu-footer">
                <Button variant="outline" onClick={() => { setContextTarget(archiveTargetMenu); setArchiveTargetMenu(null); }}>返回</Button>
                <Button variant="ghost" onClick={() => setArchiveTargetMenu(null)}>取消</Button>
              </DialogFooter>
            </>
          )}
        </DialogContent>
      </Dialog>

      <Dialog open={openedFile !== null} onOpenChange={(open) => { if (!open) setOpenedFile(null); }}>
        <DialogContent className="open-file-dialog" showCloseButton={false}>
          {openedEntry && openedFile && (
            <>
              <DialogHeader>
                <DialogTitle>已默认打开</DialogTitle>
                <DialogDescription>{openedEntry.name}</DialogDescription>
              </DialogHeader>
              <div className="open-file-summary">
                <span className={`file-icon file-icon--${openedEntry.fileType ?? 'generic'}`}><FileIcon entry={openedEntry} /></span>
                <div>
                  <strong>{openedEntry.name}</strong>
                  <span>{entryMeta(openedEntry)}</span>
                  <span>{getPath(manager, openedFile.side === 'left' ? manager.panes.left.currentDirectoryId : manager.panes.right.currentDirectoryId)}</span>
                </div>
              </div>
              <p className="open-file-note">这是网页演示，文件内容不会读取或上传。</p>
              <DialogFooter>
                <Button type="button" onClick={() => setOpenedFile(null)}>关闭</Button>
              </DialogFooter>
            </>
          )}
        </DialogContent>
      </Dialog>

      <Dialog open={nameDialog !== null} onOpenChange={(open) => { if (!open) setNameDialog(null); }}>
        <DialogContent className="name-dialog" showCloseButton={false}>
          <form onSubmit={submitName}>
            <DialogHeader>
              <DialogTitle>{nameDialog?.mode === 'rename' ? '重命名条目' : '新建文件夹'}</DialogTitle>
              <DialogDescription>
                {nameDialog?.mode === 'rename' ? '输入新的名称，原演示条目的内容保持不变。' : `将在 ${activePath} 中创建演示文件夹。`}
              </DialogDescription>
            </DialogHeader>
            <div className="name-field">
              <label htmlFor="entry-name">名称</label>
              <Input
                id="entry-name"
                autoFocus
                value={nameValue}
                onChange={(event) => { setNameValue(event.target.value); setNameError(''); }}
                aria-invalid={Boolean(nameError)}
                aria-describedby={nameError ? 'name-error' : undefined}
              />
              {nameError && <p id="name-error" role="alert">{nameError}</p>}
            </div>
            <DialogFooter>
              <Button type="button" variant="outline" onClick={() => setNameDialog(null)}>取消</Button>
              <Button type="submit">确认</Button>
            </DialogFooter>
          </form>
        </DialogContent>
      </Dialog>

      <AlertDialog open={deleteRequest !== null} onOpenChange={(open) => { if (!open) setDeleteRequest(null); }}>
        <AlertDialogContent>
          <AlertDialogHeader>
            <AlertDialogTitle>删除所选条目？</AlertDialogTitle>
            <AlertDialogDescription>
              将从演示数据中删除 {deleteRequest?.entryIds.length ?? 0} 项及其子内容。刷新页面或重置演示数据即可恢复。
            </AlertDialogDescription>
          </AlertDialogHeader>
          <AlertDialogFooter>
            <AlertDialogCancel>取消</AlertDialogCancel>
            <Button variant="destructive" onClick={confirmDelete}>删除</Button>
          </AlertDialogFooter>
        </AlertDialogContent>
      </AlertDialog>
    </Toaster>
  );
}
