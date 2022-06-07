/* Generated by Cython 3.0.0a10 */

#ifndef __PYX_HAVE__renderer
#define __PYX_HAVE__renderer

#include "Python.h"

#ifndef __PYX_HAVE_API__renderer

#ifndef __PYX_EXTERN_C
  #ifdef __cplusplus
    #define __PYX_EXTERN_C extern "C"
  #else
    #define __PYX_EXTERN_C extern
  #endif
#endif

#ifndef DL_IMPORT
  #define DL_IMPORT(_T) _T
#endif

__PYX_EXTERN_C int astrid_load_instrument(void);
__PYX_EXTERN_C int astrid_reload_instrument(void);
__PYX_EXTERN_C int astrid_render_event(void);
__PYX_EXTERN_C int astrid_get_messages(void);
__PYX_EXTERN_C int astrid_get_instrument_status(int *);
__PYX_EXTERN_C int astrid_get_info(size_t *, int *, int *);
__PYX_EXTERN_C int astrid_buffer_count(int *);
__PYX_EXTERN_C int astrid_copy_buffer(lpbuffer_t *);
__PYX_EXTERN_C int astrid_copy_adc(lpbuffer_t *);

#endif /* !__PYX_HAVE_API__renderer */

/* WARNING: the interface of the module init function changed in CPython 3.5. */
/* It now returns a PyModuleDef instance instead of a PyModule instance. */

#if PY_MAJOR_VERSION < 3
PyMODINIT_FUNC initrenderer(void);
#else
/* WARNING: Use PyImport_AppendInittab("renderer", PyInit_renderer) instead of calling PyInit_renderer directly from Python 3.5 */
PyMODINIT_FUNC PyInit_renderer(void);

#if PY_VERSION_HEX >= 0x03050000 && (defined(__GNUC__) || defined(__clang__) || defined(_MSC_VER) || (defined(__cplusplus) && __cplusplus >= 201402L))
#if defined(__cplusplus) && __cplusplus >= 201402L
[[deprecated("Use PyImport_AppendInittab(\"renderer\", PyInit_renderer) instead of calling PyInit_renderer directly.")]] inline
#elif defined(__GNUC__) || defined(__clang__)
__attribute__ ((__deprecated__("Use PyImport_AppendInittab(\"renderer\", PyInit_renderer) instead of calling PyInit_renderer directly."), __unused__)) __inline__
#elif defined(_MSC_VER)
__declspec(deprecated("Use PyImport_AppendInittab(\"renderer\", PyInit_renderer) instead of calling PyInit_renderer directly.")) __inline
#endif
static PyObject* __PYX_WARN_IF_PyInit_renderer_INIT_CALLED(PyObject* res) {
  return res;
}
#define PyInit_renderer() __PYX_WARN_IF_PyInit_renderer_INIT_CALLED(PyInit_renderer())
#endif
#endif

#endif /* !__PYX_HAVE__renderer */
