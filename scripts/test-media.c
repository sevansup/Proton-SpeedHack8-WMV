/* Minimal declarations for the stable GStreamer 1.x / GLib C ABI.
 * Proton ships these shared libraries without development headers. */
#include <stdint.h>
#include <string.h>
typedef void GstElement;
typedef void GstBuffer;
typedef void GstPad;
typedef void GstCaps;
typedef void GstBus;
typedef void GstMessage;
typedef void GstStructure;
typedef void *gpointer;
typedef char gchar;
typedef struct { unsigned domain; int code; char *message; } GError;
#define FALSE 0
#define TRUE 1
#define GST_PAD_LINK_OK 0
#define GST_SECOND UINT64_C(1000000000)
#define GST_STATE_PLAYING 4
#define GST_STATE_NULL 1
#define GST_MESSAGE_ERROR 2
#define GST_MESSAGE_EOS 1
#define GST_BIN(p) (p)
#define G_CALLBACK(p) ((void (*)(void))(p))
#define g_str_has_prefix(s,p) (strncmp((s),(p),strlen(p)) == 0)
extern void gst_init(int *, char ***);
extern GstElement *gst_pipeline_new(const char *);
extern GstElement *gst_element_factory_make(const char *, const char *);
extern void g_object_set(void *, const char *, ...);
extern unsigned long g_signal_connect_data(void *, const char *, void (*)(void), void *, void *, int);
#define g_signal_connect(o,n,c,d) g_signal_connect_data(o,n,c,d,NULL,0)
extern void gst_bin_add_many(void *, GstElement *, ...);
extern int gst_element_link(GstElement *, GstElement *);
extern GstPad *gst_element_get_static_pad(GstElement *, const char *);
extern GstCaps *gst_pad_get_current_caps(GstPad *);
extern const GstStructure *gst_caps_get_structure(const GstCaps *, unsigned);
extern const char *gst_structure_get_name(const GstStructure *);
extern int gst_pad_link(GstPad *, GstPad *);
extern void gst_caps_unref(GstCaps *);
extern void gst_object_unref(void *);
extern GstBus *gst_element_get_bus(GstElement *);
extern int gst_element_set_state(GstElement *, int);
extern GstMessage *gst_bus_timed_pop_filtered(GstBus *, uint64_t, unsigned);
extern const GstStructure *gst_message_get_structure(GstMessage *);
extern void gst_message_parse_error(GstMessage *, GError **, char **);
extern void gst_message_unref(GstMessage *);
extern void g_error_free(GError *);
extern void g_free(void *);
#include <stdio.h>
static GstElement *video, *audio;
static unsigned video_frames, audio_frames;
static void handoff(GstElement *sink, GstBuffer *buffer, GstPad *pad, gpointer data) {
    (*(unsigned *)data)++;
}
static void pad_added(GstElement *demux, GstPad *pad, gpointer data) {
    GstCaps *caps = gst_pad_get_current_caps(pad);
    const char *type = gst_structure_get_name(gst_caps_get_structure(caps, 0));
    GstElement *target = g_str_has_prefix(type, "video/") ? video :
                         g_str_has_prefix(type, "audio/") ? audio : NULL;
    if (target) {
        GstPad *sink = gst_element_get_static_pad(target, "sink");
        if (gst_pad_link(pad, sink) != GST_PAD_LINK_OK) fprintf(stderr, "Cannot link %s\n", type);
        gst_object_unref(sink);
    }
    gst_caps_unref(caps);
}
int main(int argc, char **argv) {
    gst_init(&argc, &argv);
    if (argc != 2) return 2;
    GstElement *pipeline = gst_pipeline_new("wmv-test");
    GstElement *source = gst_element_factory_make("filesrc", NULL);
    GstElement *demux = gst_element_factory_make("asfdemux", NULL);
    video = gst_element_factory_make("avdec_wmv3", NULL);
    audio = gst_element_factory_make("avdec_wmav2", NULL);
    GstElement *vsink = gst_element_factory_make("fakesink", NULL);
    GstElement *asink = gst_element_factory_make("fakesink", NULL);
    if (!source || !demux || !video || !audio || !vsink || !asink) {
        fprintf(stderr, "Missing WMV/ASF decoder component\n"); return 3;
    }
    g_object_set(source, "location", argv[1], NULL);
    g_object_set(vsink, "sync", FALSE, "async", FALSE, "signal-handoffs", TRUE, NULL);
    g_object_set(asink, "sync", FALSE, "async", FALSE, "signal-handoffs", TRUE, NULL);
    g_signal_connect(vsink, "handoff", G_CALLBACK(handoff), &video_frames);
    g_signal_connect(asink, "handoff", G_CALLBACK(handoff), &audio_frames);
    g_signal_connect(demux, "pad-added", G_CALLBACK(pad_added), NULL);
    gst_bin_add_many(GST_BIN(pipeline), source, demux, video, audio, vsink, asink, NULL);
    if (!gst_element_link(source, demux) || !gst_element_link(video, vsink) || !gst_element_link(audio, asink)) return 4;
    GstBus *bus = gst_element_get_bus(pipeline);
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    GstMessage *msg = gst_bus_timed_pop_filtered(bus, 120*GST_SECOND, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);
    int result = 0;
    if (!msg) { fprintf(stderr, "Timeout\n"); result = 5; }
    else if (gst_message_get_structure(msg) && strcmp(gst_structure_get_name(gst_message_get_structure(msg)), "GstMessageError") == 0) {
        GError *error = NULL; gchar *debug = NULL;
        gst_message_parse_error(msg, &error, &debug);
        fprintf(stderr, "%s\n%s\n", error->message, debug ? debug : ""); result = 6;
        g_error_free(error); g_free(debug);
    }
    gst_element_set_state(pipeline, GST_STATE_NULL);
    printf("WMV3 video frames=%u; WMA2 audio buffers=%u\n", video_frames, audio_frames);
    if (video_frames != 677 || audio_frames == 0) result = 7;
    if (msg) gst_message_unref(msg);
    gst_object_unref(bus); gst_object_unref(pipeline);
    if (!result) puts("PASS: complete WMV3 video and WMA2 audio decode");
    return result;
}
