#!/bin/bash
set -e
HOST="http://localhost:8080"
AUTH="-u admin:admin"
BUNDLE="/home/rahma/yocto-project/build/tmp/deploy/images/raspberrypi4/my-update-bundle-raspberrypi4-20261031140906.raucb"

echo "1. Activer targettoken..."
curl -s $AUTH -X PUT "$HOST/rest/v1/system/configs/authentication.targettoken.enabled" \
  -H 'Content-Type: application/json' -d '{"value":true}' -o /dev/null -w "  -> %{http_code}\n"

echo "2. Créer la target..."
TARGET_JSON=$(curl -s $AUTH -X POST "$HOST/rest/v1/targets" \
  -H 'Content-Type: application/json' \
  -d '[{"controllerId":"rpi4-001","name":"rpi4-001"}]')
echo "$TARGET_JSON"
TOKEN=$(echo "$TARGET_JSON" | grep -o '"securityToken":"[^"]*"' | cut -d'"' -f4)
echo "  -> TOKEN = $TOKEN"

echo "3. Créer le software module..."
SM_JSON=$(curl -s $AUTH -X POST "$HOST/rest/v1/softwaremodules" \
  -H 'Content-Type: application/json' \
  -d '[{"name":"my-update","version":"v1.0","type":"os"}]')
echo "$SM_JSON"
SM_ID=$(echo "$SM_JSON" | grep -o '"id":[0-9]*' | head -1 | cut -d: -f2)
echo "  -> SM_ID = $SM_ID"

echo "4. Uploader le bundle..."
curl -s $AUTH -X POST "$HOST/rest/v1/softwaremodules/$SM_ID/artifacts" \
  -F "file=@$BUNDLE" -o /dev/null -w "  -> %{http_code}\n"

echo "5. Créer le distribution set..."
DS_JSON=$(curl -s $AUTH -X POST "$HOST/rest/v1/distributionsets" \
  -H 'Content-Type: application/json' \
  -d '[{"name":"my-dist","version":"v1.0","type":"os"}]')
echo "$DS_JSON"
DS_ID=$(echo "$DS_JSON" | grep -o '"id":[0-9]*' | head -1 | cut -d: -f2)
echo "  -> DS_ID = $DS_ID"

echo "6. Ajouter le module au set..."
curl -s $AUTH -X POST "$HOST/rest/v1/distributionsets/$DS_ID/assignedSM" \
  -H 'Content-Type: application/json' -d "[{\"id\":$SM_ID}]" -o /dev/null -w "  -> %{http_code}\n"

echo "7. Assigner le set à la target..."
curl -s $AUTH -X POST "$HOST/rest/v1/targets/rpi4-001/assignedDS" \
  -H 'Content-Type: application/json' -d "{\"id\":$DS_ID,\"type\":\"forced\"}"

echo
echo "=== TOKEN de rpi4-001 (à garder) : $TOKEN ==="
