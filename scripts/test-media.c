#include <gst/gst.h>
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
        if (gst_pad_link(pad, sink) != GST_PAD_LINK_OK) g_error("Cannot link %s", type);
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
    else if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ERROR) {
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
