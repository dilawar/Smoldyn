//! server

use axum::{Router, extract::State, response::Html, routing::get};
use std::net::SocketAddr;
use std::sync::Arc;

pub struct WebServer {
    addr: SocketAddr,
    state: Arc<AppState>,
}

pub(crate) struct AppState {
    app_name: String,
    version: String,
    // TODO: some way to fetch simulation data here.
    template_engine: TemplateEngine,
}

impl AppState {
    fn render_home_page(&self) -> String {
        let context = tera::context! {
            app_name => &self.app_name,
            app_version => &self.version,
        };

        self.template_engine
            .render_index_page(&context)
            .map_err(|e| tracing::error!(e=?e, "failed to render index.html"))
            .unwrap_or("failed to render".to_string())
    }
}

struct TemplateEngine(tera::Tera);

impl Default for TemplateEngine {
    fn default() -> Self {
        let mut engine = tera::Tera::default();
        engine
            .add_raw_template("index", include_str!("./index.html"))
            .expect("failed to load templates");

        Self(engine)
    }
}

impl TemplateEngine {
    pub fn render_index_page(&self, context: &tera::Context) -> anyhow::Result<String> {
        Ok(self.0.render("index", context)?)
    }
}

impl WebServer {
    pub fn new(port: u16) -> Self {
        let addr = SocketAddr::from(([127, 0, 0, 1], port));
        let state = Arc::new(AppState {
            app_name: "Smoldyn".to_string(),
            version: env!("CARGO_PKG_VERSION").to_string(),
            template_engine: TemplateEngine::default(),
        });

        Self { addr, state }
    }

    pub async fn run(self) {
        // Build the router and attach the shared state
        let app = Router::new()
            .route("/", get(Self::root_handler))
            .route("/simdata", get(Self::simdata))
            .route("/version", get(Self::version))
            .with_state(self.state);

        // Bind the TCP listener
        let listener = tokio::net::TcpListener::bind(&self.addr).await.unwrap();

        println!("Server running on http://{}", self.addr);

        // Start the server
        axum::serve(listener, app).await.unwrap();
    }

    // generate the web-page here.
    async fn root_handler(State(state): State<Arc<AppState>>) -> Html<String> {
        Html(state.render_home_page())
    }

    // fetch simdata
    async fn simdata(State(_state): State<Arc<AppState>>) -> String {
        let time = std::time::SystemTime::now();

        format!("t={time:?}")
    }

    async fn version(State(state): State<Arc<AppState>>) -> String {
        state.version.clone()
    }
}
