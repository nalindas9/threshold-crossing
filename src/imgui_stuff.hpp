#include <stack>

namespace imgui_stuff
{
	static void glfw_error_callback(int error, const char* description)
	{
		fprintf(stderr, "Glfw Error %d: %s\n", error, description);
	}

	inline ImFont* font_normal = nullptr;
	inline ImFont* font_normal_small = nullptr;
	inline ImFont* font_bold = nullptr;
	inline ImFont* font_italic = nullptr;
	inline ImFont* font_bolditalic = nullptr;
	inline std::stack<float> scale_padding(std::deque<float>{ 20.0f });
	inline std::stack<float> axis_handle_width(std::deque<float>{ 10.0f });
	inline std::stack<float> axis_height(std::deque<float>{ 30.0f });
	inline std::stack<int> axis_sig_figs(std::deque<int>{ 3 });
	inline std::stack<float> axis_ideal_num_marks(std::deque<float>{ 5.0f });
	inline std::stack<ImFont**> axis_font(std::deque<ImFont**>{ &font_normal_small });
	inline std::stack<ImU32> axis_font_color;
	inline std::stack<float> cursor_selection_width(std::deque<float>{ 20.0f });

	// Runs tasks that need to be performed after opengl / imgui initialization, such as loading of fonts.
	// Call this function in main() after ImGui::CreateContext, ImGui_ImplGlfw_InitForOpenGL and 
	// ImGui_ImplOpenGL3_Init have been called, but before the main loop starts.
	// Provided paths must point to existing TTF files.
	inline void init(
		const char* font_normal_path,
		const char* font_bold_path,
		const char* font_italic_path,
		const char* font_bolditalic_path,
		float normal_size, float small_size)
	{
		axis_font_color.push(ImGui::GetColorU32({ 1, 1, 1, 1 }));
		ImGuiIO& io = ImGui::GetIO();
		font_normal = io.Fonts->AddFontFromFileTTF(font_normal_path, normal_size);
		font_normal_small = io.Fonts->AddFontFromFileTTF(font_normal_path, small_size);
		font_bold = io.Fonts->AddFontFromFileTTF(font_bold_path, normal_size);
		font_italic = io.Fonts->AddFontFromFileTTF(font_italic_path, normal_size);
		font_bolditalic = io.Fonts->AddFontFromFileTTF(font_bolditalic_path, normal_size);
	}

	inline GLFWwindow* ImguiStartup()
	{
		// Setup window
		glfwSetErrorCallback(glfw_error_callback);
		if (!glfwInit())
			return nullptr;

		// GL 3.0 + GLSL 130
		const char* glsl_version = "#version 130";
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
		glfwWindowHint(GLFW_SAMPLES, 4);
		glfwWindowHint(GLFW_REFRESH_RATE, 120);

		GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);
		if (window == NULL)
			return nullptr;
		glfwMakeContextCurrent(window);
		glfwSwapInterval(1); // Enable/Disable vsync

		bool err = glewInit() != 0;

		if (err)
		{
			fprintf(stderr, "Failed to initialize OpenGL loader!\n");
			return nullptr;
		}

		glEnable(GL_MULTISAMPLE); //use multiple fragment samples in computing the final color of a pixel
		glEnable(GL_LINE_SMOOTH); //draw lines with correct filtering. Otherwise, draw aliased lines
		glEnable(GL_BLEND); //blend the computed fragment color values with the values in the color buffers

		/*
		glHint parameters:
			-GL_LINE_SMOOTH_HINT : Indicates the sampling quality of antialiased lines. If a larger filter function is applied, hinting GL_NICEST can result in more pixel fragments being generated during rasterization.
			-GL_NICEST : The most correct, or highest quality, option should be chosen
		*/
		glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //Blend function (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) : rendering antialiased points and lines in arbitrary order.

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsClassic();

		// Setup Platform/Renderer bindings
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init(glsl_version);

		return window;
	}

	inline void MainLoopBegin(ImGuiWindowFlags window_flags)
	{
		// Poll and handle events (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		glfwPollEvents();

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// state for checking whether window should be open, and what size it should be
		static bool main_window_open = true;
		static int glfw_width, glfw_height;
		auto* glfw_window = glfwGetCurrentContext();
		glfwGetWindowSize(glfw_window, &glfw_width, &glfw_height);
		ImGui::SetNextWindowSize({ static_cast<float>(glfw_width), static_cast<float>(glfw_height) });
		ImGui::SetNextWindowPos({ 0, 0 });

		ImGui::Begin("MainWindow", &main_window_open, window_flags);
		ImGui::PushFont(font_normal);
	}

	inline void MainLoopEnd(GLFWwindow* window)
	{
		ImGui::PopFont();
		ImGui::End();

		// Rendering
		ImGui::Render();
		static int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
	}

	inline void ImguiShutdown(GLFWwindow* window)
	{
		// Cleanup
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		glfwDestroyWindow(window);
		glfwTerminate();
	}
}