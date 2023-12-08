#include "main.h"

MYMOD(net.kubikas3.speedovc, Speedo, 1.0, kubikas3)

BEGIN_DEPLIST()
ADD_DEPENDENCY_VER(net.rusjj.aml, 1.1)
END_DEPLIST()

NEEDGAME(com.rockstargames.gtavc)

RwTexture *LoadTextureFromPNG(const char *path, const char *name)
{
    char file[512];
    int w, h, d, f;
    RwTexture *pTexture = nullptr;

    sprintf(file, "%s/%s.png", path, name);

    RwImage *pImage = RtPNGImageRead(file);

    if (!pImage)
    {
        logger->Error("Failed to read %s image", name);
    }

    RwImageFindRasterFormat(pImage, rwRASTERTYPETEXTURE, &w, &h, &d, &f);
    RwRaster *pRaster = RwRasterCreate(w, h, d, f);

    if (!pRaster)
    {
        logger->Error("Failed to create raster for %s", name);
    }

    RwRasterSetFromImage(pRaster, pImage);
    pTexture = RwTextureCreate(pRaster);

    if (!pTexture)
    {
        logger->Error("Failed to create texture from %s", name);
    }

    RwTextureSetName(pTexture, name);

    if ((pTexture->raster->cFormat & 0x80) == 0)
        RwTextureSetFilterMode(pTexture, rwFILTERLINEAR);
    else
        RwTextureSetFilterMode(pTexture, rwFILTERLINEARMIPLINEAR);

    RwTextureSetAddressing(pTexture, rwTEXTUREADDRESSWRAP);

    RwImageDestroy(pImage);

    return pTexture;
}

void RotateVertices(RwOpenGLVertex *pVertices, int numVertices, float angle, float cx, float cy)
{
    float c = cosf(angle);
    float s = sinf(angle);

    for (int i = 0; i < numVertices; i++)
    {
        float dx = pVertices[i].x - cx;
        float dy = pVertices[i].y - cy;

        pVertices[i].x = dx * c - dy * s + cx;
        pVertices[i].y = dx * s + dy * c + cy;
    }
}

void SetVerticesFix(RwIm2DVertex *pVerts, CRect const &rect,
                    CRGBA const &topLeftColor, CRGBA const &topRightColor, CRGBA const &bottomLeftColor, CRGBA const &bottomRightColor,
                    float tu1, float tv1, float tu2, float tv2, float tu3, float tv3, float tu4, float tv4)
{
    SetVertices(pVerts, rect, bottomLeftColor, bottomRightColor, topLeftColor, topRightColor, tu1, tv1, tu2, tv2, tu4, tv4, tu3, tv3);
}

void DrawTexture(RwTexture *pTexture, CRGBA const &color, float x, float y, float width, float height, float angle = 0)
{
    CRect rect(x, y, x + width, y + height);

    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, pTexture->raster);
    SetVerticesFix(maVertices, rect, color, color, color, color, 0, 0, 1, 0, 1, 1, 0, 1);

    if (angle != 0.0f)
    {
        RotateVertices(maVertices, 4, angle, x + width / 2, y + height / 2);
    }

    RwIm2DRenderPrimitive(rwPRIMTYPETRIFAN, maVertices, 4);
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, 0);
}

DECL_HOOK(void, RadarLoadTextures, void *self)
{
    logger->Info("Loading textures");

    logger->Info("vertices: %X", maVertices);
    logger->Info("SetVertices: %X", SetVertices);

    logger->Info("RenderStateSet: %X", RwRenderStateSet);
    logger->Info("RenderPrimitive: %X", RwIm2DRenderPrimitive);

    pDialTexture = LoadTextureFromPNG("texture/speedo", "sspeed");
    pArrowTexture = LoadTextureFromPNG("texture/speedo", "sarrow");

    logger->Info("Textures loaded successfully");

    return RadarLoadTextures(self);
}

DECL_HOOK(void, DrawBlips, void *self)
{
    uintptr_t pVehicle = FindPlayerVehicle();

    if (pVehicle)
    {
        eVehicleApperance vehicleAppearance = GetVehicleAppearance(pVehicle);

        if (vehicleAppearance == VEHICLE_APPEARANCE_AUTOMOBILE || vehicleAppearance == VEHICLE_APPEARANCE_BIKE)
        {
            CVector speedVec = *FindPlayerSpeed();

            float speedKmph = speedVec.Magnitude() * 180;
            float angle = SPEEDOMETER_MULTIPLIER * speedKmph / 180 * PI;

            uintptr_t pRadarWidget = GetRadarButton(*pGTouchscreen);
            float radarX = *(float *)(pRadarWidget + 4);
            float radarY = *(float *)(pRadarWidget + 8);
            float radarWidth = *(float *)(pRadarWidget + 12);
            float radarHeight = *(float *)(pRadarWidget + 16);

            float speedoWidth = radarWidth * 1.9f;
            float speedoHeight = radarHeight * 1.9f;
            float x = radarX + (radarWidth - speedoWidth) / 2;
            float y = radarY + (radarHeight - speedoHeight) / 2;

            DrawTexture(pDialTexture, WHITE, x, y, speedoWidth, speedoHeight);
            DrawTexture(pArrowTexture, BLACK, x, y + 2, speedoWidth, speedoHeight, -angle); // Shadow
            DrawTexture(pArrowTexture, WHITE, x, y, speedoWidth, speedoHeight, -angle);
        }
    }

    return DrawBlips(self);
}

DECL_HOOK(void, RadarShutdown, void *self)
{
    logger->Info("Unloading sprites");

    if (pDialTexture)
    {
        RwTextureDestroy(pDialTexture);
    }

    if (pArrowTexture)
    {
        RwTextureDestroy(pArrowTexture);
    }

    logger->Info("Sprites unloaded successfully");

    return RadarShutdown(self);
}

extern "C" void OnModLoad()
{
    logger->SetTag("SpeedoVC");
    void *hLibGTAVC = aml->GetLibHandle("libGTAVC.so");

    if (hLibGTAVC)
    {
        logger->Info("Speedo mod is loaded!");

        SET_TO(FindPlayerVehicle, aml->GetSym(hLibGTAVC, "_Z17FindPlayerVehiclev"));
        SET_TO(GetVehicleAppearance, aml->GetSym(hLibGTAVC, "_ZN8CVehicle20GetVehicleAppearanceEv"));
        SET_TO(FindPlayerSpeed, aml->GetSym(hLibGTAVC, "_Z15FindPlayerSpeedv"));

        // Widgets
        SET_TO(pGTouchscreen, aml->GetSym(hLibGTAVC, "GTouchscreen"));
        SET_TO(GetRadarButton, aml->GetSym(hLibGTAVC, "_ZN11Touchscreen14GetRadarButtonEv"));

        // CSprite2d
        SET_TO(maVertices, aml->GetSym(hLibGTAVC, "_ZN9CSprite2d10maVerticesE"));
        SET_TO(SetVertices, aml->GetSym(hLibGTAVC, "_ZN9CSprite2d11SetVerticesEP14RwOpenGLVertexRK5CRectRK5CRGBAS7_S7_S7_ffffffff"));

        // RenderWare
        SET_TO(RwRenderStateSet, aml->GetSym(hLibGTAVC, "_Z16RwRenderStateSet13RwRenderStatePv"));
        SET_TO(RwIm2DRenderPrimitive, aml->GetSym(hLibGTAVC, "_Z21RwIm2DRenderPrimitive15RwPrimitiveTypeP14RwOpenGLVertexi"));

        // Load PNG
        SET_TO(RtPNGImageRead, aml->GetSym(hLibGTAVC, "RtPNGImageRead"));
        SET_TO(RwImageFindRasterFormat, aml->GetSym(hLibGTAVC, "_Z23RwImageFindRasterFormatP7RwImageiPiS1_S1_S1_"));
        SET_TO(RwRasterCreate, aml->GetSym(hLibGTAVC, "_Z14RwRasterCreateiiii"));
        SET_TO(RwRasterSetFromImage, aml->GetSym(hLibGTAVC, "_Z20RwRasterSetFromImageP8RwRasterP7RwImage"));
        SET_TO(RwImageDestroy, aml->GetSym(hLibGTAVC, "_Z14RwImageDestroyP7RwImage"));
        SET_TO(RwTextureCreate, aml->GetSym(hLibGTAVC, "_Z15RwTextureCreateP8RwRaster"));
        SET_TO(RwTextureDestroy, aml->GetSym(hLibGTAVC, "_Z16RwTextureDestroyP9RwTexture"));
        SET_TO(RwTextureSetName, aml->GetSym(hLibGTAVC, "_Z16RwTextureSetNameP9RwTexturePKc"));

        // Radar hooks
        HOOK(RadarLoadTextures, aml->GetSym(hLibGTAVC, "_ZN6CRadar12LoadTexturesEv"));
        HOOK(DrawBlips, aml->GetSym(hLibGTAVC, "_ZN6CRadar9DrawBlipsEv"));
        HOOK(RadarShutdown, aml->GetSym(hLibGTAVC, "_ZN6CRadar8ShutdownEv"));
    }
    else
    {
        logger->Error("Speedo mod is not loaded :(");
        return;
    }
}
