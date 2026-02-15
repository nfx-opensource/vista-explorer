#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <dnv/vista/sdk/VIS.h>

#include <algorithm>
#include <iostream>
#include <vector>

GLFWwindow* createWindow()
{
    if( !glfwInit() )
    {
        std::cerr << "Failed to initialize GLFW\n";
        return nullptr;
    }

    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 5 );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

    GLFWwindow* window = glfwCreateWindow( 1920, 1080, "Vista explorer", nullptr, nullptr );
    if( !window )
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent( window );
    glfwSwapInterval( 1 );

    return window;
}

bool initializeImGui( GLFWwindow* window )
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsDark();

    const char* glsl_version = "#version 330";
    ImGui_ImplGlfw_InitForOpenGL( window, true );
    ImGui_ImplOpenGL3_Init( glsl_version );

    return true;
}

void cleanup( GLFWwindow* window )
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow( window );
    glfwTerminate();
}

void renderVistaExplorer()
{
    using namespace dnv::vista::sdk;

    ImGui::SetNextWindowSize( ImVec2( 800, 600 ), ImGuiCond_FirstUseEver );
    ImGui::Begin( "Gmod Viewer" );

    // Header with version selector
    ImGui::SeparatorText( "VIS version" );

    const auto& vis = VIS::instance();
    const auto& versions = vis.versions();
    auto currentVersion = vis.latest();

    // Version selector
    static int versionIndex = static_cast<int>( versions.size() ) - 1;
    if( ImGui::BeginCombo( "VIS Version", VisVersions::toString( versions[versionIndex] ).data() ) )
    {
        for( size_t i = 0; i < versions.size(); ++i )
        {
            bool isSelected = ( versionIndex == static_cast<int>( i ) );
            if( ImGui::Selectable( VisVersions::toString( versions[i] ).data(), isSelected ) )
            {
                versionIndex = static_cast<int>( i );
            }
            if( isSelected )
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    currentVersion = versions[versionIndex];
    const auto& gmod = vis.gmod( currentVersion );

    ImGui::SameLine();
    ImGui::Text( "| Nodes: %zu", std::distance( gmod.begin(), gmod.end() ) );

    ImGui::Separator();

    // Legend
    if( ImGui::CollapsingHeader( "Legend", ImGuiTreeNodeFlags_DefaultOpen ) )
    {
        ImGui::Indent();

        // Badge types
        ImGui::TextUnformatted( "Badge Types:" );
        ImGui::BulletText( "Green badge: Asset function or Product function code" );
        ImGui::BulletText( "Red badge: Product Type code (products that can be assigned to functions)" );

        ImGui::Spacing();

        // Node categories (from Annex C, Table C.1)
        ImGui::TextUnformatted( "Node Categories:" );
        ImGui::BulletText( "Asset function leaf: Lowest level asset functions (e.g., 411.1 Propulsion driver)" );
        ImGui::BulletText( "Asset function group: Grouping of asset functions (e.g., 411 Propulsion)" );
        ImGui::BulletText( "Product function leaf: Lowest level product functions within products" );
        ImGui::BulletText( "Product function group: Grouping of product functions" );
        ImGui::BulletText( "Product Type: Specific product that can be assigned (e.g., C101 Engine)" );
        ImGui::BulletText( "Product Selection: Group of selectable Product Types (hidden, children shown)" );

        ImGui::Spacing();

        // Colors
        ImGui::TextUnformatted( "Badge Colors:" );

        ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.0f, 0.5f, 0.0f, 1.0f ) );
        ImGui::SmallButton( "#008000" );
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::TextUnformatted( "Dark green - GROUP nodes" );

        ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.0f, 1.0f, 0.0f, 1.0f ) );
        ImGui::SmallButton( "#00ff00" );
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::TextUnformatted( "Lime green - ASSET FUNCTION LEAF" );

        ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.6f, 0.8f, 0.0f, 1.0f ) );
        ImGui::SmallButton( "#99cc00" );
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::TextUnformatted( "Yellow-green - PRODUCT FUNCTION COMPOSITION" );

        ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.8f, 1.0f, 0.8f, 1.0f ) );
        ImGui::SmallButton( "#ccffcc" );
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::TextUnformatted( "Light green - PRODUCT FUNCTION LEAF" );

        ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.9f, 0.2f, 0.2f, 1.0f ) );
        ImGui::SmallButton( "#ff0000" );
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::TextUnformatted( "Red - PRODUCT TYPE" );

        ImGui::Unindent();
    }

    ImGui::Separator();

    // Gmod Tree view
    ImGui::BeginChild( "GmodTree", ImVec2( 0, 0 ), true );

    // Recursive function to render tree nodes
    // parentCode: optional parent code to display as green badge on Product Types
    std::function<void( const GmodNode&, std::string_view )> renderNode;
    renderNode = [&]( const GmodNode& node, std::string_view parentCode = "" ) {
        // Product Types are now rendered with parent badge
        bool isProductType = node.metadata().category() == "PRODUCT" && node.metadata().type() == "TYPE";

        // Node flags
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

        if( node.children().isEmpty() )
        {
            flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        }

        // Color based on node properties
        std::string_view category = node.metadata().category();
        std::string_view type = node.metadata().type();

        ImVec4 badgeBg;
        ImVec4 badgeText;

        // #ff0000 -Red filled for Product Selections (CS1, CS3, etc.)
        if( node.isProductSelection() )
        {
            badgeBg = ImVec4( 0.9f, 0.2f, 0.2f, 1.0f );
            badgeText = ImVec4( 1.0f, 1.0f, 1.0f, 1.0f );
        }

        // #008000 - Dark green for all GROUP types
        else if( type == "GROUP" )
        {
            badgeBg = ImVec4( 0.0f, 0.5f, 0.0f, 1.0f );
            badgeText = ImVec4( 0.0f, 0.0f, 0.0f, 1.0f );
        }

        // #00ff00 - Lime green for ASSET FUNCTION LEAF
        else if( category == "ASSET FUNCTION" && type == "LEAF" )
        {
            badgeBg = ImVec4( 0.0f, 1.0f, 0.0f, 1.0f );
            badgeText = ImVec4( 0.0f, 0.0f, 0.0f, 1.0f );
        }

        // #99cc00 - Yellow-green for PRODUCT FUNCTION COMPOSITION
        else if( category == "PRODUCT FUNCTION" && type == "COMPOSITION" )
        {
            badgeBg = ImVec4( 0.6f, 0.8f, 0.0f, 1.0f );
            badgeText = ImVec4( 0.0f, 0.0f, 0.0f, 1.0f );
        }

        // #ccffcc - Light green for PRODUCT FUNCTION LEAF
        else if( category == "PRODUCT FUNCTION" && type == "LEAF" )
        {
            badgeBg = ImVec4( 0.8f, 1.0f, 0.8f, 1.0f );
            badgeText = ImVec4( 0.0f, 0.0f, 0.0f, 1.0f );
        }
        // #008000 - Default green for everything else
        else
        {
            badgeBg = ImVec4( 0.0f, 1.0f, 0.0f, 1.0f );
            badgeText = ImVec4( 0.0f, 0.0f, 0.0f, 1.0f );
        }

        // Render tree arrow and indentation manually
        ImGui::AlignTextToFramePadding();

        char treeId[256];
        snprintf( treeId, sizeof( treeId ), "##tree_%s", node.code().data() );

        bool nodeOpen = false;
        if( !node.children().isEmpty() )
        {
            nodeOpen = ImGui::TreeNodeEx( treeId, ImGuiTreeNodeFlags_None );
            ImGui::SameLine();
        }
        else
        {
            ImGui::TreeNodeEx( treeId, ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen );
            ImGui::SameLine();
        }

        // Render parent badge (green) if provided
        if( !parentCode.empty() )
        {
            ImGui::PushStyleVar( ImGuiStyleVar_FrameRounding, 12.0f );
            ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 8.0f, 2.0f ) );
            ImGui::PushStyleColor( ImGuiCol_Button, badgeBg );
            ImGui::PushStyleColor( ImGuiCol_ButtonHovered, badgeBg );
            ImGui::PushStyleColor( ImGuiCol_ButtonActive, badgeBg );
            ImGui::PushStyleColor( ImGuiCol_Text, badgeText );

            ImGui::SmallButton( parentCode.data() );

            ImGui::PopStyleColor( 4 );
            ImGui::PopStyleVar( 2 );
            ImGui::SameLine();
        }

        // Render main badge (red for Product Types, green for others)
        ImVec4 mainBadgeBg = isProductType ? ImVec4( 0.9f, 0.2f, 0.2f, 1.0f ) : badgeBg;
        ImVec4 mainBadgeText = isProductType ? ImVec4( 1.0f, 1.0f, 1.0f, 1.0f ) : badgeText;

        ImGui::PushStyleVar( ImGuiStyleVar_FrameRounding, 12.0f );
        ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 8.0f, 2.0f ) );
        ImGui::PushStyleColor( ImGuiCol_Button, mainBadgeBg );
        ImGui::PushStyleColor( ImGuiCol_ButtonHovered, mainBadgeBg );
        ImGui::PushStyleColor( ImGuiCol_ButtonActive, mainBadgeBg );
        ImGui::PushStyleColor( ImGuiCol_Text, mainBadgeText );

        ImGui::SmallButton( node.code().data() );

        ImGui::PopStyleColor( 4 );
        ImGui::PopStyleVar( 2 );

        // Check if node has a Product Type - show red badge for PRODUCT FUNCTION and ASSET FUNCTION
        auto productTypeOpt = node.productType();
        if( productTypeOpt.has_value() && ( category == "PRODUCT FUNCTION" || category == "ASSET FUNCTION" ) )
        {
            const auto* productTypeNode = productTypeOpt.value();
            ImGui::SameLine();
            ImGui::PushStyleVar( ImGuiStyleVar_FrameRounding, 12.0f );
            ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 8.0f, 2.0f ) );
            ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.9f, 0.2f, 0.2f, 1.0f ) );
            ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4( 0.9f, 0.2f, 0.2f, 1.0f ) );
            ImGui::PushStyleColor( ImGuiCol_ButtonActive, ImVec4( 0.9f, 0.2f, 0.2f, 1.0f ) );
            ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 1.0f, 1.0f, 1.0f, 1.0f ) );

            ImGui::SmallButton( productTypeNode->code().data() );

            ImGui::PopStyleColor( 4 );
            ImGui::PopStyleVar( 2 );
        }

        ImGui::SameLine();

        // Use commonName if available, otherwise fallback to name
        if( node.metadata().commonName().has_value() )
        {
            ImGui::TextUnformatted( node.metadata().commonName().value().data() );
        }
        else
        {
            ImGui::TextUnformatted( node.metadata().name().data() );
        }

        // Render children recursively
        if( nodeOpen && !node.children().isEmpty() )
        {
            for( const auto* child : node.children() )
            {
                // If child is a Product Selection (CS1, CS2, etc.), skip it but render its Product Type children
                if( child->isProductSelection() )
                {
                    // Render Product Types with current node's code as parent badge
                    if( !child->children().isEmpty() )
                    {
                        for( const auto* grandchild : child->children() )
                        {
                            renderNode( *grandchild, node.code() );
                        }
                    }
                }
                // If child is a Product Type, render it with current node's code as parent badge
                else if( child->metadata().category() == "PRODUCT" && child->metadata().type() == "TYPE" )
                {
                    // If current node is also a Product Type, use its parent code, otherwise use current code
                    std::string_view badgeToShow = isProductType ? parentCode : node.code();
                    renderNode( *child, badgeToShow );
                }
                else
                {
                    // Normal children rendered without parent badge
                    renderNode( *child, "" );
                }
            }
            ImGui::TreePop();
        }
    };

    // Start from root node - sort first level children lexicographically
    const auto& rootNode = gmod.rootNode();
    if( !rootNode.children().isEmpty() )
    {
        std::vector<const dnv::vista::sdk::GmodNode*> sortedChildren;
        for( const auto* child : rootNode.children() )
        {
            sortedChildren.push_back( child );
        }
        // Natural sort: extract numeric prefix for proper ordering (000a < 100a < 1000a)
        std::sort(
            sortedChildren.begin(),
            sortedChildren.end(),
            []( const dnv::vista::sdk::GmodNode* a, const dnv::vista::sdk::GmodNode* b ) {
                auto extractNumber = []( std::string_view code ) -> int {
                    int num = 0;
                    for( char c : code )
                    {
                        if( c >= '0' && c <= '9' )
                        {
                            num = num * 10 + ( c - '0' );
                        }
                        else
                        {
                            break;
                        }
                    }
                    return num;
                };
                int numA = extractNumber( a->code() );
                int numB = extractNumber( b->code() );
                if( numA != numB )
                {
                    return numA < numB;
                }
                return a->code() < b->code(); // Fallback to lexicographic if same number
            } );

        for( const auto* child : sortedChildren )
        {
            renderNode( *child, "" );
        }
    }

    ImGui::EndChild();
    ImGui::End();
}

int main()
{
    GLFWwindow* window = createWindow();
    if( !window )
    {
        return 1;
    }

    if( !initializeImGui( window ) )
    {
        cleanup( window );
        return 1;
    }

    while( !glfwWindowShouldClose( window ) )
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        renderVistaExplorer();

        ImGui::Render();

        int display_w, display_h;
        glfwGetFramebufferSize( window, &display_w, &display_h );
        glViewport( 0, 0, display_w, display_h );
        glClearColor( 0.1f, 0.1f, 0.12f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT );

        ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );

        glfwSwapBuffers( window );
    }

    cleanup( window );
    return 0;
}
