# Application Programming Interface

VCA is written primarily in C++ and x86 assembly language. This API is wholly defined within :file:`vcaLib.h` in the source/lib/ folder of our source tree.  All of the functions and variables and enumerations meant to be used by the end-user are present in this header.

- `vca_analyzer_open(vca_param param)`

    > Create a new analyzer handler, all parameters from vca_param are copied. The returned pointer is then passed to all of the functions pertaining to this analyzer. Since `vca_param` is copied internally,  the user may release their copy after allocating the analyzer. Changes made to their copy of the param structure have no affect on the analyzer after it has been allocated.

- `vca_result vca_analyzer_push(vca_analyzer *enc, vca_frame *frame)`

    > Push a frame to the analyzer and start the analysis. Note that only the pointers will be copied but no ownership of the memory is transferred to the library. The caller must make sure that the pointers are valid until the frame was analyzed. Once a results for a frame was pulled the library will not use pointers anymore. This may block until there is a slot available to work on. The number of frames that will be processed in parallel can be set using nrFrameThreads.

- `bool vca_result_available(vca_analyzer *enc)`

    > Check if a result is available to pull.

- `vca_result vca_analyzer_pull_frame_result(vca_analyzer *enc, vca_frame_results *result)`

    > Pull a result from the analyzer. This may block until a result is available. Use `vca_result_available()` if you want to only check if a result is ready.

- `void vca_analyzer_close(vca_analyzer *enc)`

    > Finally, the analyzer must be closed in order to free all of its resources. An analyzer that has been flushed cannot be restarted and reused. Once `vca_analyzer_close()` has been called, the analyzer handle must be discarded.
