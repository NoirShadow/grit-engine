/* Copyright (c) David Cunningham and the Grit Game Engine project 2015
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "gfx_gasoline.h"
#include "gfx_gasoline_parser.h"
#include "gfx_gasoline_type_system.h"
#include "gfx_gasoline_backend_gsl.h"
#include "gfx_gasoline_backend_cg.h"
#include "gfx_gasoline_backend_glsl.h"


static GfxGslTypeMap make_global_fields (GfxGslAllocator &alloc)
{
    GfxGslTypeMap m;

    m["cameraPos"] = alloc.makeType<GfxGslFloatType>(3);
    m["fovY"] = alloc.makeType<GfxGslFloatType>(1);
    m["proj"] = alloc.makeType<GfxGslFloatMatrixType>(4,4);
    m["time"] = alloc.makeType<GfxGslFloatType>(1);
    m["view"] = alloc.makeType<GfxGslFloatMatrixType>(4,4);
    m["viewportSize"] = alloc.makeType<GfxGslFloatType>(2);
    m["viewProj"] = alloc.makeType<GfxGslFloatMatrixType>(4,4);
    m["world"] = alloc.makeType<GfxGslFloatMatrixType>(4,4);
    m["worldView"] = alloc.makeType<GfxGslFloatMatrixType>(4,4);
    m["worldViewProj"] = alloc.makeType<GfxGslFloatMatrixType>(4,4);

    m["particleAmbient"] = alloc.makeType<GfxGslFloatType>(3);
    m["sunlightDiffuse"] = alloc.makeType<GfxGslFloatType>(3);
    m["sunlightDirection"] = alloc.makeType<GfxGslFloatType>(3);
    m["sunlightSpecular"] = alloc.makeType<GfxGslFloatType>(3);

    m["fogColour"] = alloc.makeType<GfxGslFloatType>(3);
    m["fogDensity"] = alloc.makeType<GfxGslFloatType>(1);
    m["hellColour"] = alloc.makeType<GfxGslFloatType>(3);
    m["skyCloudColour"] = alloc.makeType<GfxGslFloatType>(3);
    m["skyCloudCoverage"] = alloc.makeType<GfxGslFloatType>(1);
    m["skyGlareHorizonElevation"] = alloc.makeType<GfxGslFloatType>(1);
    m["skyGlareSunDistance"] = alloc.makeType<GfxGslFloatType>(1);
    m["sunAlpha"] = alloc.makeType<GfxGslFloatType>(1);
    m["sunColour"] = alloc.makeType<GfxGslFloatType>(3);
    m["sunDirection"] = alloc.makeType<GfxGslFloatType>(3);
    m["sunFalloffDistance"] = alloc.makeType<GfxGslFloatType>(1);
    m["sunSize"] = alloc.makeType<GfxGslFloatType>(1);

    m["skyDivider1"] = alloc.makeType<GfxGslFloatType>(1);
    m["skyDivider2"] = alloc.makeType<GfxGslFloatType>(1);
    m["skyDivider3"] = alloc.makeType<GfxGslFloatType>(1);
    m["skyDivider4"] = alloc.makeType<GfxGslFloatType>(1);

    m["skyColour0"] = alloc.makeType<GfxGslFloatType>(3);
    m["skyColour1"] = alloc.makeType<GfxGslFloatType>(3);
    m["skyColour2"] = alloc.makeType<GfxGslFloatType>(3);
    m["skyColour3"] = alloc.makeType<GfxGslFloatType>(3);
    m["skyColour4"] = alloc.makeType<GfxGslFloatType>(3);
    m["skyColour5"] = alloc.makeType<GfxGslFloatType>(3);

    m["skySunColour0"] = alloc.makeType<GfxGslFloatType>(3);
    m["skySunColour1"] = alloc.makeType<GfxGslFloatType>(3);
    m["skySunColour2"] = alloc.makeType<GfxGslFloatType>(3);
    m["skySunColour3"] = alloc.makeType<GfxGslFloatType>(3);
    m["skySunColour4"] = alloc.makeType<GfxGslFloatType>(3);

    m["envCubeCrossFade"] = alloc.makeType<GfxGslFloatType>(1);
    m["envCubeMipmaps0"] = alloc.makeType<GfxGslFloatType>(1);
    m["envCubeMipmaps1"] = alloc.makeType<GfxGslFloatType>(1);

    m["colourGradeLut"] = alloc.makeType<GfxGslFloatTextureType>(3);
    m["envCube0"] = alloc.makeType<GfxGslFloatTextureCubeType>();
    m["envCube1"] = alloc.makeType<GfxGslFloatTextureCubeType>();
    m["fadeDitherMap"] = alloc.makeType<GfxGslFloatTextureType>(2);
    m["shadowPcfNoiseMap"] = alloc.makeType<GfxGslFloatTextureType>(2);
    m["shadowMap0"] = alloc.makeType<GfxGslFloatTextureType>(2);
    m["shadowMap1"] = alloc.makeType<GfxGslFloatTextureType>(2);
    m["shadowMap2"] = alloc.makeType<GfxGslFloatTextureType>(2);

    return m;
}
 
std::map<std::string, std::vector<GfxGslFunctionType*>> make_func_types (GfxGslAllocator &alloc)
{
    std::map<std::string, std::vector<GfxGslFunctionType*>> m;

    auto is = [&] (int i) -> GfxGslIntType* { return alloc.makeType<GfxGslIntType>(i); };
    auto fs = [&] (int i) -> GfxGslFloatType* { return alloc.makeType<GfxGslFloatType>(i); };
    auto tex = [&] (int i) -> GfxGslFloatTextureType* { return alloc.makeType<GfxGslFloatTextureType>(i); };

    auto *cube = alloc.makeType<GfxGslFloatTextureCubeType>();

    //GfxGslType *b = alloc.makeType<GfxGslBoolType>();
    // ts holds the set of all functions: float_n -> float_n
    std::vector<GfxGslFunctionType*> ts;
    for (unsigned i=1 ; i<=4 ; ++i)
        ts.push_back(alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(i)}, fs(i)));
    m["tan"] = ts;
    m["atan"] = ts;
    m["sin"] = ts;
    m["asin"] = ts;
    m["cos"] = ts;
    m["acos"] = ts;
    m["ddx"] = ts;  // Only available in fragment shader
    m["ddy"] = ts;  // Only available in fragment shader
    m["abs"] = ts;
    m["sqrt"] = ts;

    m["pow"] = {
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(1), fs(1)}, fs(1)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(2), fs(1)}, fs(2)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(3), fs(1)}, fs(3)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(4), fs(1)}, fs(4)),
    };

    m["strength"] = { alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(1), fs(1)}, fs(1)), };

    m["atan2"] = { alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(1), fs(1)}, fs(1)) };

    m["dot"] = {
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(3), fs(3)}, fs(1)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(2), fs(2)}, fs(1)),
    };
    m["normalize"] = { alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(3)}, fs(3)) };
    m["reflect"] = { alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(3), fs(3)}, fs(3)) };
    m["cross"] = { alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(3), fs(3)}, fs(3)) };

    m["rotate_to_world"] =
        { alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(3)}, fs(3)) };

    m["transform_to_world"] =
        { alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(3)}, fs(3)) };

    // ts holds the set of all functions: (float_n) -> float_1
    ts.clear();
    for (unsigned i=1 ; i<=4 ; ++i)
        ts.push_back(alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(i)}, fs(1)));
    m["length"] = ts;

    // ts holds the set of all functions: (float_n, float_n, float_n) -> float_n
    ts.clear();
    for (unsigned i=1 ; i<=4 ; ++i)
        ts.push_back(alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(i), fs(i), fs(i)}, fs(i)));
    m["clamp"] = ts;

    m["Float"] = {
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{is(1)}, fs(1)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(1)}, fs(1)),
    };
    m["Float2"] = {
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{is(1)}, fs(2)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(1)}, fs(2)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{is(2)}, fs(2)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(2)}, fs(2)),

        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(1), fs(1)}, fs(2))
    };
    m["Float3"] = {
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{is(1)}, fs(3)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(1)}, fs(3)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{is(3)}, fs(3)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(3)}, fs(3)),

        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(2), fs(1)}, fs(3)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(1), fs(2)}, fs(3)),

        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(1), fs(1), fs(1)}, fs(3)),
    };

    m["Float4"] = {
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{is(1)}, fs(4)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(1)}, fs(4)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{is(4)}, fs(4)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(4)}, fs(4)),

        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(3), fs(1)}, fs(4)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(1), fs(3)}, fs(4)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(2), fs(2)}, fs(4)),

        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(1), fs(1), fs(2)}, fs(4)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(1), fs(2), fs(1)}, fs(4)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(2), fs(1), fs(1)}, fs(4)),

        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(1), fs(1), fs(1), fs(1)}, fs(4))
    };

    // ts holds the set of all functions: (float_n, float_n, float1) -> float_n
    ts.clear();
    for (unsigned i=1 ; i<=4 ; ++i)
        ts.push_back(alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(i), fs(i), fs(1)}, fs(i)));
    m["lerp"] = ts;


    // ts holds the set of all functions: (float_n, float_n) -> float_n
    ts.clear();
    for (unsigned i=1 ; i<=4 ; ++i)
        ts.push_back(alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(i), fs(i)}, fs(i)));
    m["max"] = ts;
    m["min"] = ts;

    // ts holds the set of all functions: (float_n, float_1) -> float_n
    ts.clear();
    for (unsigned i=1 ; i<=4 ; ++i)
        ts.push_back(alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(i), fs(1)}, fs(i)));
    m["mod"] = ts;

    ts.clear();
    for (unsigned w=2 ; w<=4 ; ++w) {
        for (unsigned h=2 ; h<=4 ; ++h) {
            GfxGslType *m = alloc.makeType<GfxGslFloatMatrixType>(w, h);
            ts.push_back(alloc.makeType<GfxGslFunctionType>(GfxGslTypes{m, fs(w)}, fs(h)));
        }
    }
    m["mul"] = ts;

    m["sample"] = {
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{tex(2), fs(2)}, fs(4)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{tex(3), fs(3)}, fs(4)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{cube, fs(3)}, fs(4)),
    };
    m["sampleGrad"] = {
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{tex(2), fs(2), fs(2), fs(2)}, fs(4)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{tex(3), fs(3), fs(3), fs(3)}, fs(4)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{cube, fs(3), fs(3), fs(3)}, fs(4)),
    };

    m["sampleLod"] = {
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{tex(2), fs(2), fs(1)}, fs(4)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{tex(3), fs(3), fs(1)}, fs(4)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{cube, fs(3), fs(1)}, fs(4)),
    };

    m["pma_decode"] = {
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(4)}, fs(4)),
    };

    m["gamma_decode"] = {
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(1)}, fs(1)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(2)}, fs(2)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(3)}, fs(3)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(4)}, fs(4)),
    };

    m["gamma_encode"] = {
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(1)}, fs(1)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(2)}, fs(2)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(3)}, fs(3)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(4)}, fs(4)),
    };

    return m;
}



static GfxGslType *to_type (GfxGslAllocator &alloc, GfxGslParamType t)
{
    switch (t) {
        case GFX_GSL_INT1: return alloc.makeType<GfxGslIntType>(1);
        case GFX_GSL_INT2: return alloc.makeType<GfxGslIntType>(2);
        case GFX_GSL_INT3: return alloc.makeType<GfxGslIntType>(3);
        case GFX_GSL_INT4: return alloc.makeType<GfxGslIntType>(4);
        case GFX_GSL_FLOAT1: return alloc.makeType<GfxGslFloatType>(1);
        case GFX_GSL_FLOAT2: return alloc.makeType<GfxGslFloatType>(2);
        case GFX_GSL_FLOAT3: return alloc.makeType<GfxGslFloatType>(3);
        case GFX_GSL_FLOAT4: return alloc.makeType<GfxGslFloatType>(4);
        case GFX_GSL_FLOAT_TEXTURE1: return alloc.makeType<GfxGslFloatTextureType>(1);
        case GFX_GSL_FLOAT_TEXTURE2: return alloc.makeType<GfxGslFloatTextureType>(2);
        case GFX_GSL_FLOAT_TEXTURE3: return alloc.makeType<GfxGslFloatTextureType>(3);
        case GFX_GSL_FLOAT_TEXTURE4: return alloc.makeType<GfxGslFloatTextureType>(4);
        case GFX_GSL_FLOAT_TEXTURE_CUBE: return alloc.makeType<GfxGslFloatTextureCubeType>();
    }
    EXCEPTEX << ENDL;
}

static GfxGslTypeMap make_mat_fields (GfxGslAllocator &alloc, const GfxGslParams &params)
{
    GfxGslTypeMap r;
    for (auto pair : params)
        r[pair.first] = to_type(alloc, pair.second);
    return r;
}

void gfx_gasoline_check (const std::string &vert_prog,
                         const std::string &dangs_prog,
                         const std::string &additional_prog,
                         const GfxGslParams &params)
{
    GfxGslAllocator alloc;
    GfxGslContext ctx = {
        alloc, make_func_types(alloc), make_global_fields(alloc),
        make_mat_fields(alloc, params), {}
    };

    GfxGslAst *vert_ast;
    GfxGslTypeSystem vert_ts(ctx, GFX_GSL_VERTEX, GfxGslTypeMap());

    try {
        vert_ast = gfx_gasoline_parse(alloc, vert_prog);
        vert_ts.inferAndSet(vert_ast);
    } catch (const Exception &e) {
        EXCEPT << "Vertex shader: " << e << ENDL;
    }


    GfxGslAst *dangs_ast;
    GfxGslTypeSystem dangs_ts(ctx, GFX_GSL_DANGS, vert_ts.getVars());

    try {
        dangs_ast = gfx_gasoline_parse(alloc, dangs_prog);
        dangs_ts.inferAndSet(dangs_ast);
    } catch (const Exception &e) {
        EXCEPT << "DANGS shader: " << e << ENDL;
    }

    GfxGslAst *additional_ast;
    GfxGslTypeSystem additional_ts(ctx, GFX_GSL_COLOUR_ALPHA, vert_ts.getVars());

    try {
        additional_ast = gfx_gasoline_parse(alloc, additional_prog);
        additional_ts.inferAndSet(additional_ast);
    } catch (const Exception &e) {
        EXCEPT << "Additional shader: " << e << ENDL;
    }
}

static GfxGasolineResult gfx_gasoline_compile_colour (GfxGslBackend backend,
                                                      const std::string &vert_prog,
                                                      const std::string &colour_prog,
                                                      const GfxGslParams &params,
                                                      const GfxGslUnboundTextures &ubt,
                                                      bool flat_z)
{
    GfxGslAllocator alloc;
    GfxGslContext ctx = {
        alloc, make_func_types(alloc), make_global_fields(alloc),
        make_mat_fields(alloc, params), ubt
    };

    GfxGslAst *vert_ast;
    GfxGslTypeSystem vert_ts(ctx, GFX_GSL_VERTEX, GfxGslTypeMap());

    try {
        vert_ast = gfx_gasoline_parse(alloc, vert_prog);
        vert_ts.inferAndSet(vert_ast);
    } catch (const Exception &e) {
        EXCEPT << "Vertex shader: " << e << ENDL;
    }


    GfxGslAst *frag_ast;
    GfxGslTypeSystem frag_ts(ctx, GFX_GSL_COLOUR_ALPHA, vert_ts.getVars());

    try {
        frag_ast = gfx_gasoline_parse(alloc, colour_prog);
        frag_ts.inferAndSet(frag_ast);
    } catch (const Exception &e) {
        EXCEPT << "Fragment shader: " << e << ENDL;
    }

    std::string vert_out;
    std::string frag_out;
    switch (backend) {
        case GFX_GSL_BACKEND_CG:
        gfx_gasoline_unparse_cg(ctx, &vert_ts, vert_ast, vert_out,
                                &frag_ts, frag_ast, frag_out, flat_z);
        break;

        case GFX_GSL_BACKEND_GLSL:
        gfx_gasoline_unparse_glsl(ctx, &vert_ts, vert_ast, vert_out,
                                  &frag_ts, frag_ast, frag_out, flat_z);
        break;
    }

    return {vert_out, frag_out};
}

GfxGasolineResult gfx_gasoline_compile_hud (GfxGslBackend backend,
                                               const std::string &vert_prog,
                                               const std::string &colour_prog,
                                               const GfxGslParams &params,
                                               const GfxGslUnboundTextures &ubt)
{
    return gfx_gasoline_compile_colour(backend, vert_prog, colour_prog, params, ubt, false);
}

GfxGasolineResult gfx_gasoline_compile_wire_frame (GfxGslBackend backend,
                                               const std::string &vert_prog,
                                               const GfxGslParams &params)
{
    std::string colour_prog = "out.colour = Float3(1, 1, 1);\n";
    return gfx_gasoline_compile_colour(backend, vert_prog, colour_prog, params, {}, false);
}

GfxGasolineResult gfx_gasoline_compile_sky (GfxGslBackend backend,
                                               const std::string &vert_prog,
                                               const std::string &colour_prog,
                                               const GfxGslParams &params,
                                               const GfxGslUnboundTextures &ubt)
{
    return gfx_gasoline_compile_colour(backend, vert_prog, colour_prog, params, ubt, true);
}

GfxGasolineResult gfx_gasoline_compile_first_person (GfxGslBackend backend,
                                                     const std::string &vert_prog,
                                                     const std::string &dangs_prog,
                                                     const std::string &additional_prog,
                                                     const GfxGslParams &params,
                                                     const GfxGslUnboundTextures &ubt)
{
    GfxGslAllocator alloc;
    GfxGslContext ctx = {
        alloc, make_func_types(alloc), make_global_fields(alloc),
        make_mat_fields(alloc, params), ubt
    };

    GfxGslAst *vert_ast;
    GfxGslTypeSystem vert_ts(ctx, GFX_GSL_VERTEX, GfxGslTypeMap());

    try {
        vert_ast = gfx_gasoline_parse(alloc, vert_prog);
        vert_ts.inferAndSet(vert_ast);
    } catch (const Exception &e) {
        EXCEPT << "Vertex shader: " << e << ENDL;
    }


    GfxGslAst *dangs_ast;
    GfxGslTypeSystem dangs_ts(ctx, GFX_GSL_DANGS, vert_ts.getVars());

    try {
        dangs_ast = gfx_gasoline_parse(alloc, dangs_prog);
        dangs_ts.inferAndSet(dangs_ast);
    } catch (const Exception &e) {
        EXCEPT << "DANGS shader: " << e << ENDL;
    }

    GfxGslAst *additional_ast;
    GfxGslTypeSystem additional_ts(ctx, GFX_GSL_COLOUR_ALPHA, vert_ts.getVars());

    try {
        additional_ast = gfx_gasoline_parse(alloc, additional_prog);
        additional_ts.inferAndSet(additional_ast);
    } catch (const Exception &e) {
        EXCEPT << "Additional shader: " << e << ENDL;
    }

    std::string vert_out;
    std::string frag_out;
    switch (backend) {
        case GFX_GSL_BACKEND_CG:
        gfx_gasoline_unparse_first_person_cg(ctx, &vert_ts, vert_ast,
                                             &dangs_ts, dangs_ast,
                                             &additional_ts, additional_ast,
                                             vert_out, frag_out);
        break;

        case GFX_GSL_BACKEND_GLSL:
        gfx_gasoline_unparse_first_person_glsl(ctx, &vert_ts, vert_ast,
                                               &dangs_ts, dangs_ast,
                                               &additional_ts, additional_ast,
                                               vert_out, frag_out);
        break;
    }

    return {vert_out, frag_out};
}

