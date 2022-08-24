%{
** Copyright (C) 2021 The Qt Company Ltd.
** SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
%}

% Create mapping matrixes to convert ambisonic to different speaker layouts supported in Qt Multimedia
%
% This uses the ADT octave framework from https://bitbucket.org/ambidecodertoolbox/adt/src/master/
% to generate conversion matrixes between ambisonic formats and various speaker configurations
%
% We're generating two band conversion matrices with a cutoff frequency of 380 Herz. Filtering low
% and high frequencies differently is important to get a decent spatial reproduction. For details
% see the "Is my decoder Ambisonic?" paper (https://ambisonics.dreamhosters.com/BLaH3.pdf)
%
function adt_generate_qt()
    % assume speakers are 2 meters from listener. This should avoid near-field
    % effects and should work for most room setups
    radius = 2;

    [outfile,msg] = fopen("qambisonicdecoderdata_p.h",'w');
    fprintf(outfile, "// Copyright (C) 2016 The Qt Company Ltd.\n");
    fprintf(outfile, "//SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only\n");
    fprintf(outfile, "#ifndef QAMBISONICDECODERDATA_P_H\n");
    fprintf(outfile, "#define QAMBISONICDECODERDATA_P_H\n\n");
    fprintf(outfile, "#include <qtspatialaudioglobal_p.h>\n\n");
    fprintf(outfile, "//  W A R N I N G\n");
    fprintf(outfile, "//  -------------\n");
    fprintf(outfile, "//\n");
    fprintf(outfile, "// This file is not part of the Qt API.  It exists purely as an\n");
    fprintf(outfile, "// implementation detail.  This header file may change from version to\n");
    fprintf(outfile, "// version without notice, or even be removed.\n");
    fprintf(outfile, "//\n");
    fprintf(outfile, "// We mean it.\n");
    fprintf(outfile, "//\n\n");
    fprintf(outfile, "// This file is generated by the matlab/octave file adt_generate_qt.m\n");
    fprintf(outfile, "// using the Ambisonic Decoder Toolbox (https://bitbucket.org/ambidecodertoolbox/adt/src/master/)\n");
    fprintf(outfile, "\n\n");
    fprintf(outfile, "QT_BEGIN_NAMESPACE\n\n");

    % cover top/bottom for surround
    imag_speakers = [0,0,radius; 0,0,-radius];

    % https://www.dolby.com/us/en/guide/surround-sound-speaker-setup/5-1-setup.html
    % Dolby 5.1 -- F: 22-30, S: 110-120, C in same plane as F
    S = ambi_spkr_array(...
        '5dot0', ...
        'AER', 'DDM', ...
        'L',  [  30, 0, radius], ...
        'R',  [ -30, 0, radius], ...
        'C',  [   0, 0, radius], ...
        'Ls', [ 110, 0, radius], ...
        'Rs', [-110, 0, radius] ...
    );
    createDecoders(S, imag_speakers, outfile);

    S.lfeRow = 4;
    S.name = "5dot1";
    createDecoders(S, imag_speakers, outfile);

    % https://www.dolby.com/us/en/guide/surround-sound-speaker-setup/7-1-setup.html
    % Dolby 7.1 F: 22-30, S: 90-110, B: 135-150
    S = ambi_spkr_array(...
        ... % array name
        '7dot0', ...
        ... % coordinate codes, unit codes
        ... % Azimuth, Elevation, Radius; Degrees, Degrees, Meters
        'AER', 'DDM', ...
        ... % speaker name, [azimuth, elevation, radius]
        'L',  [  30, 0, radius], ...
        'R',  [ -30, 0, radius], ...
        'C',  [   0, 0, radius], ...
        'Ls', [  90, 0, radius], ...
        'Rs', [ -90, 0, radius], ...
        'Lb', [ 150, 0, radius], ...
        'Rb', [-150, 0, radius] ...
    );
    createDecoders(S, imag_speakers, outfile);

    S.lfeRow = 4;
    S.name = "7dot1";
    createDecoders(S, imag_speakers, outfile);

    fprintf(outfile, "QT_END_NAMESPACE\n\n");
    fprintf(outfile, "#endif\n\n");
    fclose(outfile);
end

% remove rounding errors
function m = trimMatrix(m)
    for i = 1:rows(m)
        for j = 1:columns(m)
            if (abs(m(i, j)) < 1e-4)
                m(i, j) = 0;
            endif
        endfor
    endfor
end

function writeLFERow(outfile, m, suffix)
    if (strcmp(suffix, 'hf'))
        fprintf(outfile, "0.0f, "); % no need for High frequency data on the LFE channel
    else
        fprintf(outfile, "0.5f, ");
    endif
    for i = 2:columns(m)
        fprintf(outfile, "0.0f, ");
    endfor
    fprintf(outfile, "// LFE\n");
end

function [n, m] = getnm(l)
% Computes spherical harmonic degree and order from Ambisonic Channel Number.
    n = floor(sqrt(l));
    m = l-n.^2-n;
end

function channels = normalizeSN3D(channels)
    for i = 1:columns(channels)
        [n, m] = getnm(i-1);
        if (m == 0)
            factor = 1;
        else
            factor = sqrt(2 * factorial(n - abs(m)) / (factorial(n + abs(m))));
        endif
        % There's an adjustment factor of 1.5 here, to match the volumes generated by the matrices
        % here with what we get from a simple stereo decoding. The factor is heuristically determined
        channels(i) *= factor*1.5;
    endfor
end

function writeMatrix(outfile, level, S, M, suffix)
    m = trimMatrix(M);
    hasLFE = isfield(S, "lfeRow");
    r = rows(m);
    c = columns(m);
    if (hasLFE)
        r = r + 1;
    endif
    fprintf(outfile, "// Decoder matrix for %s, ambisonic level %d\n", S.name, level);
    fprintf(outfile, "static constexpr float decoderMatrix_%s_%d_%s[%d*%d] = {\n", S.name, level, suffix, r, c);
    for i = 1:rows(S.id)
        channels = normalizeSN3D(m(i, :))
        fprintf(outfile, "%ff, ", channels);
        fprintf(outfile, "// %s\n", S.id(i, 1){1});
        if (hasLFE && S.lfeRow == i + 1)
            writeLFERow(outfile, m, suffix);
        endif
    endfor
    fprintf(outfile, "};\n\n");
end

function createOneDecoder(S, imag_speakers, outfile, level)
    ambi_order = ambi_channel_definitions_convention(level, 'ambix2011')
    [D,S,M,C] = ambi_run_allrad(S, ambi_order, imag_speakers, [S.name '_' int2str(level)], false, "amb", 1, 3);
    writeMatrix(outfile, level, S, M.lf, "lf");
    m = ambi_apply_gamma(M.hf, D.hf_gains, C);
    writeMatrix(outfile, level, S, m, "hf");
end

function createDecoders(S, imag_speakers, outfile)
    for level = [1:3]
        createOneDecoder(S, imag_speakers, outfile, level)
    endfor
end
