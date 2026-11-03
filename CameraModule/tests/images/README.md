# Inspelade bilder

Bilder från banan som testas i `test_recorded.py`. Varje bild körs genom `tape_mask`,
`strip_lines` och `calcdistance` och jämförs med sitt facit.

## Lägga till en bild

1. Spara en bild från kameran på Pi:n, till exempel i `camera.py` direkt efter `capture_array()`:

   ```python
   cv2.imwrite("rak_vag.png", frame)
   ```

2. Lägg bilden här, till exempel `rak_vag.png`.
3. Skapa `rak_vag.json` bredvid med facit:

   ```json
   {"lines": 2, "distance_left": 120, "distance_right": 150, "tolerance": 10}
   ```

   - `lines`: antal tejpremsor i bilden.
   - `distance_left` och `distance_right`: avstånd i pixlar från den gröna punkten till
     närmaste linje på varje sida. Utelämna dem om bilden inte ska testa avstånd.
   - `tolerance`: hur många pixlar fel som är okej. Utan den gäller 10.

Mät facit i bilden, till exempel i ett bildprogram, inte genom att köra koden. Annars testar
testet bara att koden gör som den redan gör.

Bra bilder att ha: rak väg, kurva åt båda hållen, stopplinje, korsning, och samma vägar i
olika ljus. Spara som png, inte jpg, så att bilden inte ändras av komprimering.
