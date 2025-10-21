/**
 * @license
 * Copyright 2024 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */
import { IDragStrategy } from '../interfaces/i_draggable.js';
import { Coordinate } from '../utils.js';
import { RenderedWorkspaceComment } from '../comments.js';
export declare class CommentDragStrategy implements IDragStrategy {
    private comment;
    private startLoc;
    private workspace;
    constructor(comment: RenderedWorkspaceComment);
    isMovable(): boolean;
    startDrag(): void;
    drag(newLoc: Coordinate): void;
    endDrag(): void;
    private fireMoveEvent;
    revertDrag(): void;
}
//# sourceMappingURL=comment_drag_strategy.d.ts.map