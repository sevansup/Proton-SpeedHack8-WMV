#!/usr/bin/env python3
"""Decode a supplied video through the selected runtime's GStreamer libraries."""
import ctypes as C
import json
import sys

gst = C.CDLL('libgstreamer-1.0.so.0')
obj = C.CDLL('libgobject-2.0.so.0')
def api(lib, name, result, *args):
    fn = getattr(lib, name)
    fn.restype, fn.argtypes = result, list(args)
    return fn
ptr, uint = C.c_void_p, C.c_uint
api(gst, 'gst_init', None, ptr, ptr)(None, None)
launch = api(gst, 'gst_parse_launch', ptr, C.c_char_p, C.POINTER(ptr))
get_bus = api(gst, 'gst_element_get_bus', ptr, ptr)
set_state = api(gst, 'gst_element_set_state', C.c_int, ptr, C.c_int)
pop = api(gst, 'gst_bus_timed_pop_filtered', ptr, ptr, C.c_uint64, uint)
get_struct = api(gst, 'gst_message_get_structure', ptr, ptr)
struct_name = api(gst, 'gst_structure_get_name', C.c_char_p, ptr)
get_by_name = api(gst, 'gst_bin_get_by_name', ptr, ptr, C.c_char_p)
class GError(C.Structure):
    _fields_ = [('domain', uint), ('code', C.c_int), ('message', C.c_char_p)]
pipeline = api(gst, 'gst_pipeline_new', ptr, C.c_char_p)(b'test')
make = api(gst, 'gst_element_factory_make', ptr, C.c_char_p, C.c_char_p)
source, decoder, sink = [make(n, n) for n in (b'filesrc', b'decodebin', b'fakesink')]
if not all((source, decoder, sink)):
    raise SystemExit('Missing required element')
prop = api(obj, 'g_object_set', None, ptr, C.c_char_p)
prop(source, b'location', C.c_char_p(sys.argv[1].encode()), ptr())
prop(sink, b'sync', C.c_int(0), b'signal-handoffs', C.c_int(1), ptr())
add = api(gst, 'gst_bin_add', C.c_int, ptr, ptr)
for element in (source, decoder, sink):
    assert add(pipeline, element)
assert api(gst, 'gst_element_link', C.c_int, ptr, ptr)(source, decoder)
connect = api(obj, 'g_signal_connect_data', C.c_ulong, ptr, C.c_char_p, ptr, ptr, ptr, uint)
@C.CFUNCTYPE(None, ptr, ptr, ptr)
def pad_added(element, pad, data):
    target = api(gst, 'gst_element_get_static_pad', ptr, ptr, C.c_char_p)(sink, b'sink')
    api(gst, 'gst_pad_link', C.c_int, ptr, ptr)(pad, target)
connect(decoder, b'pad-added', pad_added, None, None, 0)
error = ptr()
count = 0
@C.CFUNCTYPE(None, ptr, ptr, ptr, ptr)
def handoff(sink, buffer, pad, data):
    global count
    count += 1
api(obj, 'g_signal_connect_data', C.c_ulong, ptr, C.c_char_p, ptr, ptr, ptr, uint)(sink, b'handoff', handoff, None, None, 0)
bus = get_bus(pipeline)
if set_state(pipeline, 4) == 0:
    raise SystemExit('Cannot start pipeline')
message = pop(bus, 120 * 1000000000, 3)
if not message:
    set_state(pipeline, 1)
    raise SystemExit('Decode timed out')
structure = get_struct(message)
if structure and struct_name(structure) == b'GstMessageError':
    debug = C.c_char_p()
    api(gst, 'gst_message_parse_error', None, ptr, C.POINTER(ptr), C.POINTER(C.c_char_p))(message, C.byref(error), C.byref(debug))
    detail = C.cast(error, C.POINTER(GError)).contents.message.decode()
    set_state(pipeline, 1)
    raise SystemExit(detail + '\n' + (debug.value or b'').decode())
set_state(pipeline, 1)
print(json.dumps({'decoded_frames': count, 'end_of_stream': True}))
if count < 1:
    raise SystemExit('No frames decoded')
