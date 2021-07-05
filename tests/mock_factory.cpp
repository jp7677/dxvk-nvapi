class MockFactory : public ResourceFactory {

    public:
    MockFactory(std::unique_ptr<IDXGIFactory> dxgiFactoryMock, std::unique_ptr<Vulkan> vulkanMock, std::unique_ptr<Nvml> nvmlMock)
        : m_dxgiFactoryMock(std::move(dxgiFactoryMock)), m_vulkanMock(std::move(vulkanMock)), m_nvmlMock(std::move(nvmlMock)) {};

    Com<IDXGIFactory> CreateDXGIFactory() override {
        Com<IDXGIFactory> dxgiFactory = m_dxgiFactoryMock.get();
        return dxgiFactory;
    };

    std::unique_ptr<Vulkan> CreateVulkan() override {
        return std::move(m_vulkanMock);
    }

    std::unique_ptr<Nvml> CreateNvml() override {
        return std::move(m_nvmlMock);
    }

    private:
    std::unique_ptr<IDXGIFactory> m_dxgiFactoryMock;
    std::unique_ptr<Vulkan> m_vulkanMock;
    std::unique_ptr<Nvml> m_nvmlMock;
};
