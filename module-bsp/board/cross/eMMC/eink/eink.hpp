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

    public:
        using Size = std::pair<uint32_t, uint32_t>;
        using Position = std::pair<uint32_t, uint32_t>;

        AbstractEink() = delete;

        AbstractEink(Size requested_size, RotateMode rotate_mode, BPPMode bpp_mode);
        
        virtual Size lookupEinkSize(RotateMode rotate_mode);

        virtual void draw(Position pos, Size canvas_size, uint8_t *buf);

        virtual void setInvertColors(bool set);

        // TODO: proposition to adjust
        virtual void UpdateWaveform();
        virtual void PowerOn() = 0;
        virtual void PowerOff() = 0;
        virtual void PowerDown() = 0;
        virtual int GetTemperature() = 0;
        virtual void Renit() = 0;
        virtual void Dither() = 0;
        virtual void Draw() = 0;
        virtual void FillScreenWithColor() = 0;
        virtual void Refresh() = 0;
};