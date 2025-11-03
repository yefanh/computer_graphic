# Flight Animation TODO

## Current task: Integrate animation nodes into visitors
- [ ] Add tick state to `sgraph::GLScenegraphRenderer` and expose setter.
- [ ] Update `GLScenegraphRenderer::visitAnimationNode` to call `setTransform(currentTick)` before applying transform.
- [ ] Ensure other visitors (printer/exporter) handle animation nodes without errors (set default behavior if needed).
- [ ] Wire tick setter from render loop (placeholder stub if tick not yet available).
