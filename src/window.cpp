//----------------------------------------------------------

#include "window.hpp"

//----------------------------------------------------------


// creates multiple points (dots) in the buffer and returns its data
// points is a vec4 array where each element is the location of the point
// colors is a vec4 array where each element is the color of the point
// returns tuple with vertex array id and buffer id (in that order)
std::tuple<unsigned int, unsigned int> createPointsBuffer(const std::vector<glm::vec4>& points, const std::vector<glm::vec4>& colors)
{
    // first we insert the coordinates into the buffer
    std::vector<float> vertices;
    for(auto point : points){
        vertices.emplace_back(point[0]); // X
        vertices.emplace_back(point[1]); // Y
        vertices.emplace_back(point[2]); // Z
    }
    // then we insert the RGB
    for(auto color : colors){
        vertices.emplace_back(color[0]); //R
        vertices.emplace_back(color[1]); //G
        vertices.emplace_back(color[2]); //B
    }
    // and then we insert the alpha channel which is separated because it gets modified way more often
    for(auto color : colors){
        vertices.emplace_back(color[3]); //A
    }

    unsigned int VAID, BID;

    // create object IDs
    glGenVertexArrays(1, &VAID);
    glGenBuffers(1, &BID);

    // bind
    glBindVertexArray(VAID);
    glBindBuffer(GL_ARRAY_BUFFER, BID);

    // insert the data
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_DYNAMIC_DRAW);

    // specify the layout
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); // coords layout
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(points.size()*3*sizeof(float))); // rgb channels layout
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)(points.size()*6*sizeof(float))); // alpha channel layout
    glEnableVertexAttribArray(2);

    // unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return {VAID, BID};
}

// creates multiple points (dots) in the buffer and returns its data
// points is an vec4 array where each element is the location of the point
// color is the color of all points
// returns tuple with vertex array id and buffer id (in that order)
std::tuple<unsigned int, unsigned int> createPointsBuffer(const std::vector<glm::vec4>& points, const glm::vec4& color)
{
    std::vector<glm::vec4> colors(points.size(), color);
    return createPointsBuffer(points, colors);
}

#define CIRCLE_RESOLUTION 64

// creates a circle in the buffer and returns its data
// location is the position in 3d space where the circle is located
// rotation is the way a circle is rotated by default the circle is on the XZ plane
// returns the data over a vector
void calcCircleData(const glm::vec4& location, const glm::quat& rotation, const float& radius, std::vector<glm::vec4>& returnData)
{
    for(int i = 0; i < CIRCLE_RESOLUTION; i++){
        returnData.emplace_back( 
            // create the point of the unit circle
            glm::vec4(
                std::cos((float)std::numbers::pi * 2.f * (float)i / (float)CIRCLE_RESOLUTION), 0.f,
                std::sin((float)std::numbers::pi * 2.f * (float)i / (float)CIRCLE_RESOLUTION), 0.f
            )
            // rotate, scale and translate
            * glm::mat4(rotation) * radius + location
        );
    }
}

#define SPHERE_MERIDIAN_RESOLUTION 12
#define SPHERE_PARALLEL_RESOLUTION (1+8*2)
#define SPHERE_RESOLUTION SPHERE_MERIDIAN_RESOLUTION+SPHERE_PARALLEL_RESOLUTION

// creates a wireframe sphere in the buffer and returns its data
// location is the position in 3d space where the sphere is located
// rotation is the way a sphere is rotated by default the circle is on the XZ plane
// returns the data over a vector
void calcSphereData(const glm::vec4& location, const glm::quat& rotation, const float& radius, std::vector<glm::vec4>& returnData)
{
    for(int i = 0; i < SPHERE_MERIDIAN_RESOLUTION; i++){
        // "meridian" spheres
        glm::quat sub_rotation = 
            glm::quat(1.f, 0.f, 0.f, 0.f) 
            * glm::angleAxis(glm::radians(180.f / (float)SPHERE_MERIDIAN_RESOLUTION * (float)i), glm::vec3(1.f, 0.f, 0.f)) 
            * glm::angleAxis(glm::radians(90.f), glm::vec3(0.f, 0.f, 1.f))
            * rotation
        ;
        calcCircleData(location, sub_rotation, radius, returnData);
    }
    for(int i = 0; i < SPHERE_PARALLEL_RESOLUTION; i++){
        // "parallel" spheres
        float h = radius * (float)std::sin(glm::radians(-90.f + (i+1) * 180.f / (SPHERE_PARALLEL_RESOLUTION+1)));
        glm::vec4 sub_location = glm::vec4(0.f, h, 0.f, 0.f) * glm::mat4(rotation) + location;
        float sub_radius = std::sqrt(radius*radius - h*h);
        calcCircleData(sub_location, rotation, sub_radius, returnData);
    }
}

// Helper to display a little (?) mark which shows a tooltip when hovered.
static void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

// this function recalculates what objects are visible to the user and displays them
void recalculateVisibility(std::vector<Object>& objects, unsigned int& VBO, std::vector<bool>& hidden, float& max, float& min)
{
    // create new alpha channel data for each object
    std::vector<float> alphas;
    for(int i = 0; i < objects.size(); i++){
        if (
            (objects[i].getMagnitude() >= min && objects[i].getMagnitude() <= max)
            && hidden[i] == false
        )
            alphas.emplace_back(1.f);
        else
            alphas.emplace_back(0.f);
    }

    glBindBuffer(GL_ARRAY_BUFFER, VBO); // prepare the buffer
    void *ptr = glMapBufferRange(GL_ARRAY_BUFFER, objects.size()*sizeof(float)*6, objects.size()*sizeof(float), GL_MAP_WRITE_BIT); // get the pointer
    memcpy(ptr, alphas.data(), sizeof(float)*objects.size()); // copy the vector data
    glUnmapBuffer(GL_ARRAY_BUFFER); // invalidate pointer
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// this function recalculates mouse debug point position
void recalculateMouse(unsigned int& VBO, CoordinatesSky& sky, Camera& cam)
{
    Coordinates3D tridi = skyTo3D(sky);

    std::vector<float> coords;
    coords.emplace_back(tridi.X);
    coords.emplace_back(tridi.Y);
    coords.emplace_back(tridi.Z);

    glBindBuffer(GL_ARRAY_BUFFER, VBO); // prepare the buffer
    void *ptr = glMapBufferRange(GL_ARRAY_BUFFER, 0, sizeof(float)*3, GL_MAP_WRITE_BIT); // get the pointer
    memcpy(ptr, coords.data(), sizeof(float)*3); // copy the vector data
    glUnmapBuffer(GL_ARRAY_BUFFER); // invalidate pointer
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void updateInput(GLFWwindow* window, Camera& cam)
{
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) cam.setRotation({0,0});
    
    bool panUp = false, panLeft = false, panDown = false, panRight = false;
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) panUp = true;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) panLeft = true;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) panDown = true;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) panRight = true;
    if (panUp || panLeft || panDown || panRight)
        cam.updateRotation({1.f*panRight - 1.f*panLeft, 1.f*panUp - 1.f*panDown});
}

void windowFunction(unsigned int W, unsigned int H, std::vector<Object>& objects, Observatory& observatory)
{
    // -------------------- init glfw
    if(!glfwInit()){
        fmt::println("Error: GLFW init failed");
        return;
    }

    glfwWindowHint(GLFW_RESIZABLE, true);
    glfwWindowHint(GLFW_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_VERSION_MINOR, 3);

    GLFWwindow* window = glfwCreateWindow(W, H, ""/*"Minor Planet Center Solver 3"*/, NULL, NULL);

    glfwMakeContextCurrent(window);

    // -------------------- init glew for opengl functions
    glewExperimental = true;
    GLenum err = glewInit();
    if(err != GLEW_OK){
        glfwTerminate();
        fmt::println("Error: GLEW init failed:\n{}", (char*)glewGetErrorString(err));
        return;
    }
    
    // -------------------- debug related
    fmt::println("Info: OpenGL version {}", (char*)glGetString(GL_VERSION));
    fmt::println("Info: IMGUI version: {}", IMGUI_VERSION);
    fmt::println("Info: Vendor: {}", (char*)glGetString(GL_VENDOR));
    fmt::println("Info: Renderer name: {}", (char*)glGetString(GL_RENDERER));
    int gl_extension_num; glGetIntegerv(GL_NUM_EXTENSIONS, &gl_extension_num);
    fmt::println("Info: {} GL extensions:", gl_extension_num);
    for (int i = 0; i < gl_extension_num; i++)
        fmt::println("\t {}", (char*)glGetStringi(GL_EXTENSIONS, i));
    fmt::println("Info: Primary GLSL version: {}", (char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
    int gl_supported_glsl_version_am; glGetIntegerv(GL_NUM_SHADING_LANGUAGE_VERSIONS, &gl_supported_glsl_version_am);
    fmt::println("Info: Supported versions:");
    for (int i = 0; i < gl_supported_glsl_version_am; i++)
        fmt::println("\t {}", (char*)glGetStringi(GL_SHADING_LANGUAGE_VERSION, i));

    glDebugMessageCallback(
        []( GLenum source, GLenum type, GLuint id, GLenum severity,
            GLsizei length, const GLchar* message, const void* userParam
        ) -> void {
            if (severity == GL_DEBUG_SEVERITY_HIGH) fmt::print("Error: ");
            else if (severity == GL_DEBUG_SEVERITY_LOW || severity == GL_DEBUG_SEVERITY_HIGH) fmt::print("Warning: ");
            else return; //fmt::print("Log: "); // this prints a lot of spam
            fmt::println("[GL] type = {} ... message = {}", type, (char*)message);
        },
        nullptr
    );


    // -------------------- opengl options
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // even if this may cause performance problems, it is needed
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE); // dont draw the side of a vertex that cant be seen
    glCullFace(GL_BACK); // the back side cant be seen
    glFrontFace(GL_CCW); // front is where the points of a triangle are connected counterclockwise
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // -------------------- init imgui
    IMGUI_CHECKVERSION(); // check that version is compatible with what its used for
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

    ImGui::StyleColorsDark();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410"); //glsl version

    // -------------------- camera
    Camera cam((float)W, (float)H, 100);
    cam.setOrientation(observatory.getCoords(), std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));

    // some testing data
    // CoordinatesGeo cs = {289.84443213466, -73.0670209146797}; 
    // cam.setOrientation(cs, 1949616955);
    // auto r = cam.SkyToSkyLocal({4.2167361*15.0, 159.4272905}, 1949616955);
    // fmt::println("{} {}", r.az, r.alt);

    // -------------------- shader
    GLProgram program(g_resourcesPath+"/shaders/vertex.glsl", g_resourcesPath+"/shaders/fragment.glsl");

    // -------------------- glfw window callback functions
    struct CallbackData {
        Camera* mainCamera;
    };
    CallbackData callbackData;
    callbackData.mainCamera = &cam;
    glfwSetWindowUserPointer(window, &callbackData); // had to do this because i need camera in a callback function

    // mouse scrolling
    glfwSetScrollCallback(window, 
        [](GLFWwindow* window, double xoffset, double yoffset) -> void {
            ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset); // first call the scroll event of imgui since its windows are in front
            if (ImGui::GetIO().WantCaptureMouse) return; // check if imgui wants to capture the mouse
            CallbackData* data = static_cast<CallbackData*>(glfwGetWindowUserPointer(window)); // retrieve callback data
            if (yoffset > 0)  data->mainCamera->zoom(true);
            else data->mainCamera->zoom(false);
        }
    );

    // window resizing
    glfwSetFramebufferSizeCallback(window, 
        [](GLFWwindow* window, int frameBufferWidth, int frameBufferHeight) -> void {
            CallbackData* data = static_cast<CallbackData*>(glfwGetWindowUserPointer(window)); // retrieve callback data
            data->mainCamera->setWindowDimensions((float)frameBufferWidth, (float)frameBufferHeight);
            glViewport(0, 0, frameBufferWidth, frameBufferHeight);
        }
    );

    // ------------------- object data

    std::vector<glm::vec4> vertices;
    for(auto obj : objects){
        vertices.emplace_back( glm::vec4(
            obj.getCoords3D().X,
            obj.getCoords3D().Y,
            obj.getCoords3D().Z,
            0.f
        ));
    }

    auto [VBobjects, VAobjects] = createPointsBuffer ( vertices, glm::vec4(1.f, 0.f, 0.f, 1.f));

    // ------------------- horizon line

    std::vector<glm::vec4> points;

    calcCircleData(
        glm::vec4(0.f, 0.f, 0.f, 0.f), // location
        cam.getOrientation(),          // rotation
        0.99f,                         // radius (it is smaller than 1 so it doesnt collide with the alt/az grid)
        points
    );

    auto [VAground, VBground] = createPointsBuffer(
        points,
        glm::vec4(0.f, 1.f, 0.f, 1.f)  // color
    );

    // ------------------- alt/az grid

    points.clear(); // we can reuse this

    calcSphereData(
        glm::vec4(0.f, 0.f, 0.f, 0.f),
        cam.getOrientation(),
        1.f,
        points
    );

    auto [VAaltazGrid, VBaltazGrid] = createPointsBuffer(
        points,
        glm::vec4(0.75f, 0.f, 0.f, 1.f)
    );

    // ------------------- cursor point (debugging)

    auto [VAmouse, VBmouse] = createPointsBuffer(
        {glm::vec4(0.f, 0.f, 0.f, 0.f)}, // point location will get updated, so no need to specify it now
        {glm::vec4(0.f, 1.f, 1.f, 1.f)}
    );

    // ------------------- X Y Z points (debugging)

    auto [VAxyz, VBxyz] = createPointsBuffer(
        {
            glm::vec4(1.f, 0.f, 0.f, 0.f),
            glm::vec4(0.f, 1.f, 0.f, 0.f),
            glm::vec4(0.f, 0.f, 1.f, 0.f)
        },
        {
            glm::vec4(1.f, 0.f, 1.f, 1.f),
            glm::vec4(0.f, 1.f, 0.f, 1.f),
            glm::vec4(0.f, 0.f, 1.f, 1.f)
        }
    );

    // -------------------- window
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glPointSize(3.0f);
    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    double mouse_xpos, mouse_ypos; // cursor positions
    CoordinatesSkyLocal mouse_altaz;
    CoordinatesSky mouse_radec, mouse_hadec;
    time_t time_now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()); // current unix time
    while(!glfwWindowShouldClose(window))
    {
        // clear
        glClear(GL_COLOR_BUFFER_BIT	| GL_DEPTH_BUFFER_BIT);

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // user interactions
        glfwPollEvents();
        updateInput(window, cam);
        cam.refresh();
        program.setUniformMat4f("MVP", cam.getTransformation());
        glfwGetCursorPos(window, &mouse_xpos, &mouse_ypos); // update cursor position
        time_now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()); // update unix time
        mouse_altaz = cam.screenToSkyLocal(mouse_xpos, mouse_ypos);
        mouse_radec = cam.screenToSky(mouse_xpos, mouse_ypos, time_now);
        mouse_hadec = cam.screenToSky_HA(mouse_xpos, mouse_ypos, time_now);
        recalculateMouse(VBmouse, mouse_radec, cam);

        // draw objects
        glBindVertexArray(VAobjects);
        glDrawArrays(GL_POINTS, 0, objects.size());

        // draw alt/az grid
        glBindVertexArray(VAaltazGrid);
        GLint starts[SPHERE_RESOLUTION];
        GLsizei sizes[SPHERE_RESOLUTION];
        for(int i = 0; i < SPHERE_RESOLUTION; i++){
            starts[i] = CIRCLE_RESOLUTION * i;
            sizes[i] = CIRCLE_RESOLUTION;
        }
        glMultiDrawArrays(GL_LINE_LOOP, starts, sizes, SPHERE_RESOLUTION);
        
        // draw horizon line
        glBindVertexArray(VAground);
        glDrawArrays(GL_LINE_LOOP, 0, CIRCLE_RESOLUTION);

        // draw cursor
        glBindVertexArray(VAmouse);
        glDrawArrays(GL_POINTS, 0, 1);

        // draw xyz directions
        glBindVertexArray(VAxyz);
        glDrawArrays(GL_POINTS, 0, 3);

        // draw imgui stuff
        ImGui::ShowDemoWindow();
        { // deugging
            ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 400, main_viewport->WorkPos.y), ImGuiCond_FirstUseEver); // default location
            ImGui::SetNextWindowSize(ImVec2(200, 250), ImGuiCond_FirstUseEver); // default size

            ImGui::Begin("Debug Window");
            
            ImGui::SeparatorText("Camera");

            ImGui::Text(fmt::format("Fov: {:.2f}", cam.getFov()).c_str());

            glm::vec3 camOri = glm::eulerAngles(cam.getOrientation());
            ImGui::Text(fmt::format("Orientation:\n Roll: {:.2f}\n Pitch: {:.2f}\n Yaw: {:.2f}", glm::degrees(camOri[2]), glm::degrees(camOri[0]), glm::degrees(camOri[1])).c_str());

            CoordinatesSkyLocal camLocal = cam.getRotation();
            ImGui::Text(fmt::format("Local position:\n Altitude: {:.2f}\n Azimuth: {:.2f}", camLocal.alt, camLocal.az).c_str());

            CoordinatesGeo pos = cam.getLocation();
            ImGui::Text(fmt::format("WGS84 Location:\n Lat: {:.2f}\n Lon: {:.2f}", pos.lat, pos.lon).c_str());

            ImGui::SeparatorText("Time");

            ImGui::Text(fmt::format("Unix seconds: {:d}", time_now).c_str());

            ImGui::Text(fmt::format("UTC+0\n time: {0:%H:%M:%S}\n date: {0:%Y-%m-%d}", fmt::gmtime(time_now)).c_str());
            
            ImGui::Text(fmt::format("Local\n time: {0:%H:%M:%S}\n date: {0:%Y-%m-%d}", fmt::localtime(time_now)).c_str());

            float the_gmst = glm::degrees((float)getGMST(time_now) / g_siderealDayLength * 2.f * (float)std::numbers::pi);
            int gmst_h = the_gmst/24.f; int gmst_m = (the_gmst/24.f - gmst_h) * 60.f; int gmst_s = ((the_gmst/24.f - gmst_h) * 60.f - gmst_m) * 60.f;
            ImGui::Text(fmt::format("GMST: {:0>2d}:{:0>2d}:{:0>2d}", gmst_h, gmst_m, gmst_s).c_str());

            float the_lst = the_gmst + pos.lon;
            int lst_h = the_lst/24.f; int lst_m = (the_lst/24.f - lst_h) * 60.f; int lst_s = ((the_lst/24.f - lst_h) * 60.f - lst_m) * 60.f;
            ImGui::Text(fmt::format("LST: {:0>2d}:{:0>2d}:{:0>2d}", lst_h, lst_m, lst_s).c_str());
            
            //static CoordinatesGeo testC = observatory.getCoords();
            //ImGui::SliderFloat("Slider lat", &testC.lat, -90.f, 90.f, "lat = %.1f");
            //ImGui::SliderFloat("Slider lon", &testC.lon, 0.f, 360.f, "lon = %.1f");
            //cam.setOrientation(testC, std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));

            ImGui::SeparatorText("Mouse");

            ImGui::Text("Positions:");
            ImGui::Text(fmt::format(" X = {:.2f}\n Y = {:.2f}", mouse_xpos, mouse_ypos).c_str());
            ImGui::Text(fmt::format(" Alt = {:.2f}\n Az = {:.2f}", mouse_altaz.alt, mouse_altaz.az).c_str());
            ImGui::Text(fmt::format(" Ra = {:.2f}\n Dec = {:.2f}\n Ha = {:.2f}", mouse_radec.ra, mouse_radec.dec, mouse_hadec.ra).c_str());

            ImGui::End();
        }
        { // object selection menu
            ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y), ImGuiCond_FirstUseEver); // default location
            ImGui::SetNextWindowSize(ImVec2(400, 600), ImGuiCond_FirstUseEver); // default size

            ImGui::Begin("Objects");

            ImGui::Text(fmt::format("Total: {} objects", objects.size()).c_str());

            ImGui::SeparatorText("Filters");
            
            static float max_magnitude = 30.f, min_magnitude = 10.f; // magnitude limits
            static std::vector<bool> hidden(objects.size(), false); // what objects are visible to the user
            static std::vector<bool> selected(objects.size(), false); // what objects had the user selected

            ImGui::SetNextItemWidth(200);
            if(ImGui::DragFloatRange2("Magnitude filter", &min_magnitude, &max_magnitude, 0.1f, -40.0f, 40.0f, "Min: %.1f", "Max: %.1f", ImGuiSliderFlags_AlwaysClamp))
                recalculateVisibility(objects, VBobjects, hidden, max_magnitude, min_magnitude);
            ImGui::SameLine(); HelpMarker(
                "If an object has a magnitude outside of\n"
                "this range, it will not be displayed\n"
            );

            ImGui::SeparatorText("Select objects");

            if (ImGui::BeginTable("Select Objects", 8, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders))
            {
                // Table header
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Name");
                ImGui::TableNextColumn();
                ImGui::Text("Magnitude");
                ImGui::TableNextColumn();
                ImGui::Text("Ra");
                ImGui::TableNextColumn();
                ImGui::Text("Dec");
                ImGui::TableNextColumn();
                ImGui::Text("Az");
                ImGui::TableNextColumn();
                ImGui::Text("Alt");
                ImGui::TableNextColumn();
                ImGui::Text("Hide");
                ImGui::TableNextColumn();
                ImGui::Text("Select");
                for (int i = 0; i < objects.size(); i++)
                {
                    // (i think) this is necesarry because vector<bool> for some (probbably good but also) annoying reason cant
                    // access the adresses of individial elements in &stuff[n] fashion, so i am forced to make it work like this >:(
                    bool selected_part = selected[i];
                    bool hidden_part = hidden[i];

                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text(objects[i].getName().c_str());
                    ImGui::TableNextColumn();
                    ImGui::Text(fmt::format("{:.2f}", objects[i].getMagnitude()).c_str());
                    auto c = objects[i].getCoordsSky();
                    auto c2 = cam.SkyToSkyLocal(c, time_now);
                    ImGui::TableNextColumn();
                    ImGui::Text(fmt::format("{:.2f}", c.ra).c_str());
                    ImGui::TableNextColumn();
                    ImGui::Text(fmt::format("{:.2f}", c.dec).c_str());
                    ImGui::TableNextColumn();
                    ImGui::Text(fmt::format("{:.2f}", c2.az).c_str());
                    ImGui::TableNextColumn();
                    ImGui::Text(fmt::format("{:.2f}", c2.alt).c_str());

                    // the following checkboxes have weird names that dont get displayed, the explanation to "why do that?" is here:
                    // https://github.com/ocornut/imgui/blob/master/docs/FAQ.md#q-why-is-my-widget-not-reacting-when-i-click-on-it
                    ImGui::TableNextColumn();
                    ImGui::Checkbox(fmt::format("##{}-1", i).c_str(), &hidden_part);
                    ImGui::TableNextColumn();
                    ImGui::Checkbox(fmt::format("##{}-2", i).c_str(), &selected_part);

                    selected[i] = selected_part;
                    if (hidden[i] != hidden_part){
                        hidden[i] = hidden_part;
                        recalculateVisibility(objects, VBobjects, hidden, max_magnitude, min_magnitude);
                    }
                    
                }
                ImGui::EndTable();
            }

            ImGui::End();
        }

        // render imgui stuff
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // update and render additional platform windows (imgui docking thing)
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable){
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(window);
        glFlush();
    }

    // -------------------- deinit
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glDeleteBuffers(1, &VBobjects);
    glDeleteVertexArrays(1, &VAobjects);
    glDeleteBuffers(1, &VBground);
    glDeleteVertexArrays(1, &VAground);
    glDeleteBuffers(1, &VBaltazGrid);
    glDeleteVertexArrays(1, &VAaltazGrid);
    glDeleteBuffers(1, &VBmouse);
    glDeleteVertexArrays(1, &VAmouse);
    glDeleteBuffers(1, &VBxyz);
    glDeleteVertexArrays(1, &VAxyz);
    glfwDestroyWindow(window);
    glfwTerminate();
}
