# RaytracerLib Design Document

Living document describing the design and development of [LibRaytracer](https://github.com/stacygaudreau/raytracer) - a 3D software-rendered raytracing libary. Includes requirements, design ideas and notes from development.

This document is a supplement to the UML design documents/diagrams and was written while developing the UML. It describes various components of the software in more detail than a diagram might.

## Rendering

Multiple components make up the rendering module.

### `Renderer`

A top-level singleton instance which encapsulates all of the rendering components in an application. There should only be one of these per application instance.

- a Renderer instance is responsible for rendering a 3D Canvas into a bitmap
- (a Canvas describes a target output image based on a Camera's perspective in a given 3D scene)
- concurrent rendering via render workers
- a renderer may be used for different purposes in a desktop app, eg: not only for high quality output images, it can also be used to generate low-res previews, and will also be used to generate small in-app renders of different 3D assets the user creates/edits (eg: in a "shape editor" window)
- is more than one Renderer supposed to exist at a time, or should it be a global instance which just has preferences loaded into it? Will have to decide
- probably we could have a single Renderer existing, and then it manages things like a queue of jobs and priorities

### `RenderJobScheduler`

Manages incoming and in-progress `RenderJobs`, scheduling, dispatching and ordering them based on priorities, purpose etc.

qos_priority:
  - 0: realtime - high priority preview thumbnails/etc in actual GUI
  - 1: background  - background disk cached thumbnail generation, etc.
  - 2: offline - print quality offline rendering to disk

modes:
  1. GUI_RENDER mode
    - realtime jobs take priority over background jobs
    - offline jobs not queued at all
  2. RENDER_ONLY mode
    - final artwork rendering, blocks rest of UI
    - cancel/pause everything in progress
    - only offline priority jobs dispatched to workers

#### Scheduler algorithm receiving job

For each pixel_block_size in job:
  1. Break image into tiles
  2. For each `RenderTile`, assign a priority, preferring:
    1. higher QoS (realtime > background)
    2. lower tile pass_index (coarsest preview pass first)
    3. other stuff eg: center tiles before outer tiles in image

#### Computing priority key in scheduler

Priorities are integral scorings computed by bitwise-packing qos_priority, pass_priority/index and additional ranking into a single uint64_t for quick priority ordering.

### `RenderJobQueue`

Queue of `RenderJobs`, managed by a `RenderJobScheduler`.

### `RenderJob`

Describes a single rendering task request in the render queue. Describes a 3D world camera, canvas specification and target image to render, as well as various qualifiers (format, purpose, etc.)

- pixel_block_sizes - vector of NxN pixel sizes for progressive preview rendering orders
- output_type
  - preview: incremental preview render (fidelity increases with each render pass)
  - print: full quality single pass for printing/final artwork
  - gui: (potentially) incremental preview/thumbnail render for displaying real-time in the GUI
  - thumbnail: low priority background/gui thumbnail image generation for caching to disk etc.
- target
  - world
  - camera
  - canvas
  - image format
  - resolution
  - filename
- on_complete
  - (optional) callback to execute when the job is completed



### `RenderTile`

Rectangular region of an image to render, created from a source `RenderJob` which is broken up by the scheduler and pushed to render threads.

- job_id
- output_type
- region within final image
- samples_per_pixel (future use)
- out_buffer - output bitmap pointer
- pass_index, pixel_block_size - for multiple pass progressive rendering
- priority - computed numerical key for scheduler ordering

### `RenderJobState`

_(this may wind up just being a part of the `RenderJob` class, depending on scope)_

Provides status information about a particular `RenderJob`. eg: waiting/running status

- percent complete
- tiles complete|remaining
- est time remaining (different for incremental vs. single-pass jobs)
- time elapsed
- status (running|waiting|cancelled)
- outputmode (to_disk|to_buffer)


### `RenderPool`

Manages a pool of RenderWorkers to provide concurrent rendering. Accepts tiles to render and allows reconfiguring pool attributes, and reporting information about running jobs and other metrics.

The threads are dumb; they simply ask the scheduler for the next `RenderTile` and run it.

- set/get n_threads
- run, stop
- metrics
  - status (idle|running)
  - n_workers running/idle
  - percent (all) complete
  - tiles/pixels/sec
  - est time remaining

### `RenderWorker`

Wraps a single thread and does rendering work. Belongs to a `RenderPool`. Has a `RenderTile` assigned to it, which it carries out rendering into the output buffer until done. Relatively dumb, other than being aware of pixel block_size to render at.







