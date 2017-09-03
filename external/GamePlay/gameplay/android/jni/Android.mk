GAMEPLAY_PATH := $(call my-dir)/../../src

# external-deps
GAMEPLAY_DEPS := ../../external-deps/lib/android/$(TARGET_ARCH_ABI)

# libgameplay-deps
LOCAL_PATH := $(GAMEPLAY_DEPS)
include $(CLEAR_VARS)
LOCAL_MODULE    := libgameplay-deps
LOCAL_SRC_FILES := libgameplay-deps.a
include $(PREBUILT_STATIC_LIBRARY)

# libgameplay
include $(CLEAR_VARS)
LOCAL_PATH := $(GAMEPLAY_PATH)
LOCAL_MODULE    := libgameplay
LOCAL_SRC_FILES := \
    AbsoluteLayout.cpp \
    AIAgent.cpp \
    AIController.cpp \
    AIMessage.cpp \
    AIState.cpp \
    AIStateMachine.cpp \
    Animation.cpp \
    AnimationClip.cpp \
    AnimationController.cpp \
    AnimationTarget.cpp \
    AnimationValue.cpp \
    AudioBuffer.cpp \
    AudioController.cpp \
    AudioListener.cpp \
    AudioSource.cpp \
    BoundingBox.cpp \
    BoundingSphere.cpp \
    Bundle.cpp \
    Button.cpp \
    Camera.cpp \
    CheckBox.cpp \
    Container.cpp \
    Control.cpp \
    ControlFactory.cpp \
    Curve.cpp \
    DebugNew.cpp \
    DepthStencilTarget.cpp \
    Drawable.cpp \
    Effect.cpp \
    FileSystem.cpp \
    FlowLayout.cpp \
    Font.cpp \
    Form.cpp \
    FrameBuffer.cpp \
    Frustum.cpp \
    Game.cpp \
    Gamepad.cpp \
    HeightField.cpp \
    Image.cpp \
    ImageControl.cpp \
    Joint.cpp \
    JoystickControl.cpp \
    Label.cpp \
    Layout.cpp \
    Light.cpp \
    Logger.cpp \
    Material.cpp \
    MaterialParameter.cpp \
    MathUtil.cpp \
    Matrix.cpp \
    Mesh.cpp \
    MeshBatch.cpp \
    MeshPart.cpp \
    MeshSkin.cpp \
    Model.cpp \
    Node.cpp \
    ParticleEmitter.cpp \
    Pass.cpp \
    Perf.cpp \
    PhysicsCharacter.cpp \
    PhysicsCollisionObject.cpp \
    PhysicsCollisionShape.cpp \
    PhysicsConstraint.cpp \
    PhysicsController.cpp \
    PhysicsFixedConstraint.cpp \
    PhysicsGenericConstraint.cpp \
    PhysicsGhostObject.cpp \
    PhysicsHingeConstraint.cpp \
    PhysicsRigidBody.cpp \
    PhysicsSocketConstraint.cpp \
    PhysicsSpringConstraint.cpp \
    PhysicsVehicle.cpp \
    PhysicsVehicleWheel.cpp \
    Plane.cpp \
    Platform.cpp \
    PlatformAndroid.cpp \
    Properties.cpp \
    PropertiesRef.cpp \
    Quaternion.cpp \
    RadioButton.cpp \
    Ray.cpp \
    Rectangle.cpp \
    Ref.cpp \
    RenderState.cpp \
    RenderTarget.cpp \
    Scene.cpp \
    SceneLoader.cpp \
    ScreenDisplayer.cpp \
    Script.cpp \
    ScriptController.cpp \
    ScriptTarget.cpp \
    Slider.cpp \
    Sprite.cpp \
    SpriteBatch.cpp \
    Technique.cpp \
    Terrain.cpp \
    TerrainPatch.cpp \
    Text.cpp \
    TextBox.cpp \
    Texture.cpp \
    Theme.cpp \
    ThemeStyle.cpp \
    TileSet.cpp \
    Transform.cpp \
    Vector2.cpp \
    Vector3.cpp \
    Vector4.cpp \
    VertexAttributeBinding.cpp \
    VertexFormat.cpp \
    VerticalLayout.cpp

LOCAL_CPPFLAGS += -std=c++11 -frtti -Wno-switch-enum -Wno-switch
LOCAL_ARM_MODE := arm
LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv2 -lOpenSLES
LOCAL_CFLAGS := -D__ANDROID__ -D_FINAL -DGP_NO_LUA_BINDINGS -DGP_SCENE_VISIT_EXTENSIONS -I"../../external-deps/include"
LOCAL_ADDITIONAL_DEPENDENCIES := gameplay
LOCAL_STATIC_LIBRARIES := android_native_app_glue libgameplay-deps
include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
