#pragma once

#ifdef NANOVG_INSIDE_IMGUI_BUNDLE
static char* nvgExampleAssetPath()
{
    return "/Users/pascal/dvp/OpenSource/ImGuiWork/_Bundle/imgui_bundle/external/nanovg/nanovg/example";
}
#else
static char* nvgExampleAssetPath()
{
    return "../example";
}
#endif
