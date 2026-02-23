# v1.1.0
- Updated to Geode v5.0.0
- Added functions to decode into user buffer and read image header (by dankmeme01)

# v1.0.5
- Updated to support Geode v5.0.0-beta.1
- Ported to new Events V2

# v1.0.4
- Updated libwebp and libjxl to latest versions
- Improved performance of reading the file data on all platforms except Android

# v1.0.3
- Fixed incompatibility when using kFmtRawData format

# v1.0.2
- Fixed undefined behavior when calling `initWithImageData` with invalid data/size

# v1.0.1
- Fixed unwanted texture deletion when there's only one sprite that uses it
- Replaced giflib implementation with stb_image for better compatibility
- Added setting to allow falling back to cocos2d default PNG loading

# v1.0.0
- Initial release