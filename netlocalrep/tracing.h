#pragma once

#define WPP_CONTROL_GUIDS \
    WPP_DEFINE_CONTROL_GUID(TraceGuid,(40D79F49, 83AB, 4E6E, A00C, 0AEA9BE8D6C5),  \
        WPP_DEFINE_BIT(MAIN)                                                       \
        WPP_DEFINE_BIT(CONTEXTS)                                                   \
        WPP_DEFINE_BIT(CALLBACKS)                                                  \
        WPP_DEFINE_BIT(DRIVER) )

#define WPP_LEVEL_FLAGS_LOGGER(lvl,flags) \
           WPP_LEVEL_LOGGER(flags)

#define WPP_LEVEL_FLAGS_ENABLED(lvl, flags) \
           (WPP_LEVEL_ENABLED(flags) && WPP_CONTROL(WPP_BIT_ ## flags).Level >= lvl)

//begin_wpp config
//USEPREFIX (fatal_message, "%!STDPREFIX! %!FILE! %!FUNC! %!LINE!");
//FUNC fatal_message{LEVEL=TRACE_LEVEL_FATAL}(FLAGS, MSG, ...);
//end_wpp

//begin_wpp config
//USEPREFIX (error_message, "%!STDPREFIX! %!FILE! %!FUNC! %!LINE!");
//FUNC error_message{LEVEL=TRACE_LEVEL_ERROR}(FLAGS, MSG, ...);
//end_wpp

//begin_wpp config
//USEPREFIX (warning_message, "%!STDPREFIX! %!FILE! %!FUNC! %!LINE!");
//FUNC warning_message{LEVEL=TRACE_LEVEL_WARNING}(FLAGS, MSG, ...);
//end_wpp

//begin_wpp config
//USEPREFIX (info_message, "%!STDPREFIX! %!FILE! %!FUNC! %!LINE!");
//FUNC info_message{LEVEL=TRACE_LEVEL_INFORMATION}(FLAGS, MSG, ...);
//end_wpp

//begin_wpp config
//USEPREFIX (verbose_message, "%!STDPREFIX! %!FILE! %!FUNC! %!LINE!");
//FUNC verbose_message{LEVEL=TRACE_LEVEL_VERBOSE}(FLAGS, MSG, ...);
//end_wpp
