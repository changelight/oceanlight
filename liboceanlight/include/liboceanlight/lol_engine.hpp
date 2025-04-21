#ifndef LIBOCEANLIGHT_ENGINE_HPP_INCLUDED
#define LIBOCEANLIGHT_ENGINE_HPP_INCLUDED
#include <array>
#include <config.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <liboceanlight/lol_window.hpp>
#include <vector>
#include <vulkan/vulkan_core.h>

extern double scroll_offset, cursor_posx, cursor_posy;
namespace liboceanlight::engine
{
	using vertex = struct lol_vertex_struct
	{
		bool operator==(const lol_vertex_struct& other) const
		{
			return pos == other.pos && color == other.color &&
				   texcoord == other.texcoord;
		}

		glm::vec3 pos;
		glm::vec3 color;
		glm::vec2 texcoord;

		static VkVertexInputBindingDescription get_binding_desc()
		{
			VkVertexInputBindingDescription binding_desc {};
			binding_desc.binding = 0;
			binding_desc.stride = sizeof(lol_vertex_struct);
			binding_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			return binding_desc;
		};

		static std::array<VkVertexInputAttributeDescription, 3>
		get_attribute_descs()
		{
			std::array<VkVertexInputAttributeDescription, 3>
				attribute_descs {};

			attribute_descs[0].binding = 0;
			attribute_descs[0].location = 0;
			attribute_descs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attribute_descs[0].offset = offsetof(lol_vertex_struct, pos);

			attribute_descs[1].binding = 0;
			attribute_descs[1].location = 1;
			attribute_descs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attribute_descs[1].offset = offsetof(lol_vertex_struct, color);

			attribute_descs[2].binding = 0;
			attribute_descs[2].location = 2;
			attribute_descs[2].format = VK_FORMAT_R32G32_SFLOAT;
			attribute_descs[2].offset = offsetof(lol_vertex_struct, texcoord);
			return attribute_descs;
		};
	};
} /* namespace liboceanlight::engine */

namespace liboceanlight::models
{
	using lol_model = struct lol_model_struct
	{
		std::string name;
		std::vector<liboceanlight::engine::vertex> vertices;
		std::vector<uint32_t> indices;
		VkBuffer vertex_buffer, index_buffer;
		VkDeviceMemory vertex_buffer_mem, index_buffer_mem;
	};
}; /* namespace liboceanlight::models */

namespace liboceanlight::engine
{

	struct lol_camera
	{
		float movement_speed {0.0025f};
		float sensitivity {0.015f};
		float eye_x {0.0f}, eye_y {0.5f}, eye_z {2.0f};
		float center_x {0.0f}, center_y {0.0f}, center_z {-1.0f};
		float up_x {0.0f}, up_y {1.0f}, up_z {0.0f};
		float yaw {180.0f}, pitch {0.0f};
		glm::vec3 eye {eye_x, eye_y, eye_z};
		glm::vec3 center {center_x, center_y, center_z};
		glm::vec3 direction {glm::normalize(eye - center)};
		glm::vec3 up {0.0f, 1.0f, 0.0f};
		glm::vec3 right {glm::normalize(glm::cross(up, direction))};
	};

	struct uniform_buffer_object
	{
		glm::mat4 model {glm::mat4(1.0f)};
		glm::mat4 view {glm::mat4(1.0f)};
		glm::mat4 proj {glm::mat4(1.0f)};
	};

	using engine_data = struct lol_engine_data_struct
	{
		/* INSTANCE */
		VkDebugUtilsMessengerEXT dbg_messenger {nullptr};
		VkInstance vulkan_instance {nullptr};
#ifdef NDEBUG
		bool validation_layer_enabled {false};
#else
		bool validation_layer_enabled {true};
#endif /* NDEBUG */

		/* DEVICE */
		VkPhysicalDevice physical_device {VK_NULL_HANDLE};
		VkDevice logical_device {VK_NULL_HANDLE};
		static constexpr std::array dev_extensions {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME};
		VkPhysicalDeviceProperties device_props {};
		VkPhysicalDeviceFeatures supported_device_features {};

		/* SURFACE */
		VkSurfaceKHR window_surface {nullptr};
		VkSurfaceCapabilitiesKHR capabilities {};
		VkSurfaceFormatKHR surface_format {};

		/* QUEUE */
		uint32_t graphics_queue_index {UINT32_MAX};
		VkQueue graphics_queue {nullptr};

		/* SWAPCHAIN */
		VkSwapchainKHR swap_chain {nullptr};
		VkExtent2D swap_extent {};
		VkPresentModeKHR present_mode;
		std::vector<VkImage> images;
		std::vector<VkImageView> image_views;
		std::vector<VkFramebuffer> frame_buffers;

		/* PIPELINE */
		VkDescriptorSetLayout descriptor_set_layout {nullptr};
		VkPipelineLayout pipeline_layout {nullptr};
		VkRenderPass render_pass {nullptr};
		VkPipeline graphics_pipeline {nullptr};

		/* COMMAND */
		static constexpr int max_frames_in_flight {2};
		VkCommandPool command_pool {nullptr};
		std::array<VkCommandBuffer, max_frames_in_flight> command_buffers;

		/* TEXTURE */
		VkImage texture_img {nullptr};
		VkDeviceMemory texture_img_mem {nullptr};
		VkImageView texture_img_view {nullptr};
		VkSampler texture_sampler {nullptr};

		/* DRAW */
		int current_frame {1};
		std::array<VkSemaphore, max_frames_in_flight> signal_sems;
		std::array<VkSemaphore, max_frames_in_flight> wait_sems;
		std::array<VkFence, max_frames_in_flight> in_flight_fences;

		/* VERTEX BUFFER */
		VkBuffer vertex_buffer {nullptr};
		VkDeviceMemory vertex_buffer_mem {nullptr};

		/* INDEX BUFFER */
		VkBuffer index_buffer {nullptr};
		VkDeviceMemory index_buffer_mem {nullptr};

		/* UNIFORM BUFFER */
		std::array<VkBuffer, max_frames_in_flight> uniform_buffers;
		std::array<VkDeviceMemory, max_frames_in_flight> uniform_buffers_mem;
		std::array<void*, max_frames_in_flight> uniform_buffers_mapped;

		/* DESCRIPTOR */
		VkDescriptorPool descriptor_pool {nullptr};
		std::array<VkDescriptorSet, max_frames_in_flight> descriptor_sets;

		/* DEPTH BUFFER */
		VkImage depth_img {nullptr};
		VkDeviceMemory depth_img_mem {nullptr};
		VkImageView depth_img_view {nullptr};
		VkFormat depth_fmt {VK_FORMAT_D32_SFLOAT};

		/* VERTICES */
		std::vector<vertex> vertices;
		std::vector<uint32_t> indices;

		/* MODELS */
		std::vector<liboceanlight::models::lol_model> model_list;
	};

	void start(liboceanlight::window&, engine_data&);
	void run(liboceanlight::window&, engine_data&);
	void draw_frame(liboceanlight::window&, engine_data&, double);
	void record_cmd_buffer(engine_data&, VkCommandBuffer&, uint32_t);
	void recreate_swapchain(liboceanlight::window&, engine_data&);
	void upload_buffer(engine_data&,
					   const void*,
					   VkDeviceSize,
					   VkBufferUsageFlagBits,
					   VkBuffer&,
					   VkDeviceMemory&);
	void update_uniform_buffer(engine_data&,
							   liboceanlight::window&,
							   uint32_t,
							   double);
	void update_camera(liboceanlight::window&, float);
} /* namespace liboceanlight::engine */

namespace std
{
	template <> struct hash<liboceanlight::engine::vertex>
	{
		size_t operator()(liboceanlight::engine::vertex const& vertex) const
		{
			return ((hash<glm::vec3>()(vertex.pos) ^
					 (hash<glm::vec3>()(vertex.color) << 1)) >>
					1) ^
				   (hash<glm::vec2>()(vertex.texcoord) << 1);
		}
	};
} /* namespace std */
#endif /* LIBOCEANLIGHT_ENGINE_HPP_INCLUDED */
