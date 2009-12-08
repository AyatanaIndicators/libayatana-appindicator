#!/bin/bash

gapi2-parser app-indicator.sources.xml

cp libappindicator-api.raw libappindicator-api.xml

gapi2-fixup --api=libappindicator-api.xml --metadata=libappindicator-api.metadata

gapi2-codegen  --outdir=generated `pkg-config --cflags gtk-sharp-2.0` --generate libappindicator-api.xml

gmcs -pkg:gtk-sharp-2.0 -target:library -out:AppIndicator.dll generated/*.cs
