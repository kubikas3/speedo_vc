#include <mod/amlmod.h>
#include <mod/logger.h>
#include <mod/config.h>
#include "game_vc/rw/rwcore.h"
#include "game_vc/CRect.h"
#include "game_vc/CRGBA.h"
#include "game_vc/CVector.h"

enum eVehicleApperance
{
    VEHICLE_APPEARANCE_AUTOMOBILE = 1,
    VEHICLE_APPEARANCE_BIKE,
    VEHICLE_APPEARANCE_HELI,
    VEHICLE_APPEARANCE_BOAT,
    VEHICLE_APPEARANCE_PLANE,
};

uintptr_t (*FindPlayerVehicle)();
eVehicleApperance (*GetVehicleAppearance)(uintptr_t pVehicle);
CVector *(*FindPlayerSpeed)();

uintptr_t *pGTouchscreen;
uintptr_t (*GetRadarButton)(uintptr_t GTouchscreen);

RwOpenGLVertex *maVertices;
void (*SetVertices)(RwIm2DVertex *pVerts, CRect const &rect,
                    CRGBA const &bottomLeftColor, CRGBA const &bottomRightColor, CRGBA const &topLeftColor, CRGBA const &topRightColor,
                    float tu1, float tv1, float tu2, float tv2, float tu4, float tv4, float tu3, float tv3);

void (*RwRenderStateSet)(RwRenderState state, void *);
void (*RwIm2DRenderPrimitive)(RwPrimitiveType primType, RwOpenGLVertex *vertices, int numVertices);

RwImage *(*RtPNGImageRead)(const RwChar *imageName);
RwImage *(*RwImageFindRasterFormat)(RwImage *ipImage, RwInt32 nRasterType, RwInt32 *npWidth, RwInt32 *npHeight, RwInt32 *npDepth, RwInt32 *npFormat);
RwRaster *(*RwRasterCreate)(RwInt32 width, RwInt32 height, RwInt32 depth, RwInt32 flags);
RwRaster *(*RwRasterSetFromImage)(RwRaster *raster, RwImage *image);
RwBool *(*RwImageDestroy)(RwImage *image);
RwTexture *(*RwTextureCreate)(RwRaster *raster);
RwTexture *(*RwTextureDestroy)(RwTexture *texture);
RwTexture *(*RwTextureSetName)(RwTexture *texture, char const *name);

const float PI = 3.141592653589793f;
const float SPEEDOMETER_MULTIPLIER = 150.0f / 250.0f;
const CRGBA BLACK(0, 0, 0);
const CRGBA WHITE(255, 255, 255);

RwTexture *pDialTexture = nullptr;
RwTexture *pArrowTexture = nullptr;