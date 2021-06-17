#include <cstdint>
#include <utility>
	
class AbstractEink
{
    enum class BPPMode {
        BPP_1,
        BPP_2,
        BPP_3,
        BPP_4,
    };
    
    enum class RotateMode {
        NORMAL,
        ROTATE_90,
        ROTATE_180,
        ROTATE_270,
    };

    enum class FillingColor
    {
        BLACK = 0,
        WHITE = 0xFF
    };

    public:
        using Size = std::pair<uint32_t, uint32_t>;
        using Position = std::pair<uint32_t, uint32_t>;

        AbstractEink() = delete;

        AbstractEink(RotateMode rotate_mode, BPPMode bpp_mode);

        virtual Size getResolution() = 0;

        virtual void powerOn() = 0;

        virtual void powerOff() = 0;

        virtual void draw(Position pos, Size canvas_size, uint8_t *buf, bool invert_colors, bool refreshDeep) = 0;

        virtual void refresh() = 0;

        virtual void refreshDeep() = 0;

        virtual void fillWithColor(FillingColor color, bool refreshDeep) = 0;

    private:
        RotateMode rotate_mode;
        BPPMode bpp_mode;
};